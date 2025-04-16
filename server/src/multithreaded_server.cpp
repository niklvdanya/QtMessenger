#include "multithreaded_server.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>

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
    
    m_clients.clear();
    m_usernames.clear();
    
    qDebug() << "Сервер остановлен";
}

void MultithreadedServer::accept_connections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);
    m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            QUuid clientId = QUuid::createUuid();
            m_clients[clientId] = socket;
            m_usernames[clientId] = "Guest";
            
            emit newConnection(clientId);
            qDebug() << "Новое подключение, UUID:" << clientId;
            
            auto buffer = std::make_shared<std::vector<char>>(1024);
            socket->async_read_some(boost::asio::buffer(*buffer),
                [this, socket, buffer, clientId](const boost::system::error_code& error, 
                                               std::size_t bytes_transferred) {
                    if (!error) {
                        handle_read(socket, buffer);
                    } else {
                        m_clients.erase(clientId);
                        m_usernames.erase(clientId);
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
    QByteArray data(buffer->data(), buffer->size());
    QDataStream stream(data);
    
    Message msg;
    stream >> msg;
    
    QUuid senderId;
    for (const auto& pair : m_clients) {
        if (pair.second == socket) {
            senderId = pair.first;
            break;
        }
    }
    
    if (!senderId.isNull()) {
        m_usernames[senderId] = msg.username;
        broadcast_message(msg.text.toStdString(), senderId, msg.username);
    }
    
    socket->async_read_some(boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& error, 
                             std::size_t bytes_transferred) {
            if (!error) {
                handle_read(socket, buffer);
            }
        });
}

void MultithreadedServer::broadcast_message(const std::string& message, 
                                          QUuid senderId, 
                                          const QString& username) {
    for (const auto& pair : m_clients) {
        if (pair.first != senderId) {
            handle_write(pair.second, username.toStdString() + ":" + message);
        }
    }
    
    emit messageReceived(QString::fromStdString(message), senderId, username);
}

void MultithreadedServer::handle_write(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                     const std::string& message) {
    boost::asio::async_write(*socket, boost::asio::buffer(message),
        [this, socket](const boost::system::error_code& error, std::size_t) {
            if (error) {
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