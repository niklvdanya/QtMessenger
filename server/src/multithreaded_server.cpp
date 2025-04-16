#include "multithreaded_server.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include <mutex>

MultithreadedServer::MultithreadedServer(unsigned short port, int thread_count, QObject* parent)
    : QObject(parent)
    , m_io_context()
    , m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , m_running(false) {
    m_threads.reserve(thread_count);
}

MultithreadedServer::~MultithreadedServer() {
    stop();
}

void MultithreadedServer::start() {
    if (m_running) return;

    m_running = true;
    accept_connections();

    for (size_t i = 0; i < m_threads.capacity(); ++i) {
        m_threads.emplace_back([this]() {
            m_io_context.run();
        });
    }

    qDebug() << "Многопоточный сервер запущен на порту" << m_acceptor.local_endpoint().port();
}

void MultithreadedServer::stop() {
    if (!m_running) return;

    m_running = false;
    m_io_context.stop();

    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_threads.clear();

    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        m_clients.clear();
        m_usernames.clear();
    }

    qDebug() << "Сервер остановлен";
}

void MultithreadedServer::accept_connections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);
    m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            QUuid clientId = QUuid::createUuid();
            {
                std::lock_guard<std::mutex> lock(m_clients_mutex);
                m_clients[clientId] = socket;
                m_usernames[clientId] = "Guest";
            }

            emit newConnection(clientId);
            qDebug() << "Новое подключение, UUID:" << clientId;

            auto buffer = std::make_shared<std::vector<char>>(1024);
            socket->async_read_some(boost::asio::buffer(*buffer),
                [this, socket, buffer, clientId](const boost::system::error_code& error, 
                                               std::size_t bytes_transferred) {
                    if (!error) {
                        handle_read(socket, buffer);
                    } else {
                        {
                            std::lock_guard<std::mutex> lock(m_clients_mutex);
                            m_clients.erase(clientId);
                            m_usernames.erase(clientId);
                        }
                        emit clientDisconnected(clientId);
                        qDebug() << "Клиент отключился, UUID:" << clientId;
                    }
                });
        }

        if (m_running) {
            accept_connections();
        }
    });
}

void MultithreadedServer::handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                     std::shared_ptr<std::vector<char>> buffer) {
    QByteArray data(buffer->data(), static_cast<int>(buffer->size()));
    QDataStream stream(data);

    Message msg;
    stream >> msg;

    QUuid senderId;
    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (const auto& pair : m_clients) {
            if (pair.second == socket) {
                senderId = pair.first;
                m_usernames[senderId] = msg.username;
                break;
            }
        }
    }

    if (!senderId.isNull()) {
        broadcast_message(msg.text, senderId, msg.username);
    }

    socket->async_read_some(boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& error, 
                              std::size_t bytes_transferred) {
            if (!error) {
                handle_read(socket, buffer);
            }
        });
}

void MultithreadedServer::broadcast_message(std::string_view message, 
                                           QUuid senderId, 
                                           std::string_view username) {
    std::string formatted_message = std::string(username) + ":" + std::string(message);
    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (const auto& pair : m_clients) {
            if (pair.first != senderId) {
                handle_write(pair.second, formatted_message);
            }
        }
    }

    emit messageReceived(std::string(message), senderId, std::string(username));
}

void MultithreadedServer::handle_write(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                      const std::string& message) {
    boost::asio::async_write(*socket, boost::asio::buffer(message),
        [this, socket](const boost::system::error_code& error, std::size_t) {
            if (error) {
                std::lock_guard<std::mutex> lock(m_clients_mutex);
                for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
                    if (it->second == socket) {
                        QUuid clientId = it->first;
                        m_clients.erase(it);
                        m_usernames.erase(clientId);
                        emit clientDisconnected(clientId);
                        break;
                    }
                }
            }
        });
}