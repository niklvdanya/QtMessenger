#include "multithreaded_server.h"
#include "boost_asio_client_session.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>

MultithreadedServer::MultithreadedServer(unsigned short port, int thread_count, QObject* parent)
    : QObject(parent),
      m_io_context(),
      m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      m_clientManager(std::make_shared<ClientManager>()),
      m_messageHandler(std::make_shared<MessageHandler>(m_clientManager)),
      m_running(false) {
    m_threads.reserve(thread_count);
}

MultithreadedServer::~MultithreadedServer() {
    stop();
}

void MultithreadedServer::start(uint16_t port) {
    if (m_running) return;

    m_running = true;
    accept_connections();

    for (size_t i = 0; i < m_threads.capacity(); ++i) {
        m_threads.emplace_back([this]() {
            m_io_context.run();
        });
    }

    qDebug() << "Multithreaded server started on port" << m_acceptor.local_endpoint().port();
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

    m_clientManager->clear();

    qDebug() << "Server stopped";
}

void MultithreadedServer::accept_connections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);
    m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            handle_connection(socket);
        }

        if (m_running) {
            accept_connections();
        }
    });
}

void MultithreadedServer::handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto session = std::make_unique<BoostAsioClientSession>(socket);
    QUuid clientId = session->uuid();

    // Устанавливаем обработчики для сообщений и отключений
    session->setMessageCallback([this](const std::string& message, QUuid senderId, const std::string& username) {
        m_messageHandler->handleMessage(message, senderId, username);
        emit messageReceived(message, senderId, username);
    });

    session->setDisconnectCallback([this](QUuid clientId) {
        m_clientManager->removeClient(clientId);
        emit clientDisconnected(clientId);
        qDebug() << "Client disconnected, UUID:" << clientId;
    });

    m_clientManager->addClient(clientId, std::move(session));

    emit newConnection(clientId);
    qDebug() << "New connection, UUID:" << clientId;
}

void MultithreadedServer::handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                     std::shared_ptr<std::vector<char>> buffer) {
    socket->async_read_some(boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                QByteArray data(buffer->data(), static_cast<int>(bytes_transferred));
                QDataStream stream(data);

                Message msg;
                stream >> msg;

                qDebug() << "Received message from socket:" << QString::fromStdString(msg.username)
                         << "Text:" << QString::fromStdString(msg.text);

                QUuid senderId;
                {
                    std::lock_guard<std::mutex> lock(m_clients_mutex);
                    for (const auto& pair : m_clientManager->getClients()) {
                        auto* session = dynamic_cast<BoostAsioClientSession*>(pair.second.get());
                        if (session && session->getSocket() == socket) {
                            senderId = pair.first;
                            break;
                        }
                    }
                }

                if (!senderId.isNull()) {
                    std::string fullMessage = msg.username + ":" + msg.text;
                    qDebug() << "Broadcasting message:" << QString::fromStdString(fullMessage);
                    m_messageHandler->handleMessage(msg.text, senderId, msg.username);
                    emit messageReceived(msg.text, senderId, msg.username);
                } else {
                    qDebug() << "No sender found for socket";
                }

                handle_read(socket, buffer);
            } else {
                QUuid clientId;
                {
                    std::lock_guard<std::mutex> lock(m_clients_mutex);
                    for (const auto& pair : m_clientManager->getClients()) {
                        auto* session = dynamic_cast<BoostAsioClientSession*>(pair.second.get());
                        if (session && session->getSocket() == socket) {
                            clientId = pair.first;
                            break;
                        }
                    }
                }
                m_clientManager->removeClient(clientId);
                emit clientDisconnected(clientId);
                qDebug() << "Client disconnected, UUID:" << clientId;
            }
        });
}