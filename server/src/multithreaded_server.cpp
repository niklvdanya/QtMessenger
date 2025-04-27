#include "multithreaded_server.h"
#include "boost_asio_client_session.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>

bool acceptAllCredentials(const QString& username, const QString& password) {
    return !username.isEmpty() && !password.isEmpty();
}

MultithreadedServer::MultithreadedServer(unsigned short port, int thread_count, DatabaseManager* dbManager, QObject* parent)
    : QObject(parent),
      m_io_context(),
      m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      m_clientManager(std::make_shared<ClientManager>()),
      m_messageHandler(std::make_shared<MessageHandler>(m_clientManager)),
      m_dbManager(dbManager),
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
    auto session = std::make_unique<BoostAsioClientSession>(socket, nullptr);
    QUuid clientId = session->uuid();

    session->setReadyCallback([this, clientId]() {
        const auto& history = m_clientManager->getChatHistory();
        auto* clientSession = dynamic_cast<BoostAsioClientSession*>(
            m_clientManager->getClients().at(clientId).get());
        if (clientSession) {
            for (const auto& msg : history) {
                clientSession->sendMessage(msg);
            }
        }
    });
    
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

    auto* sessionPtr = dynamic_cast<BoostAsioClientSession*>(m_clientManager->getClients().at(clientId).get());
    auto buffer = std::make_shared<std::vector<char>>(1024);

    socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, socket, buffer, clientId, sessionPtr](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                qDebug() << "Error reading credentials:" << QString::fromStdString(error.message());
                m_clientManager->removeClient(clientId);
                return;
            }
            
            try {
        
                QByteArray rawData(buffer->data(), static_cast<int>(bytes_transferred));
                QDataStream stream(&rawData, QIODevice::ReadOnly);
                stream.setVersion(QDataStream::Qt_6_0);
                
                QString username, password;
                stream >> username >> password;
                
                if (username.isEmpty() || password.isEmpty()) {
                    m_clientManager->removeClient(clientId);
                    return;
                }

                bool validCredentials = acceptAllCredentials(username, password);
                
                if (validCredentials) {
                    sessionPtr->m_username = username.toStdString();
                    sessionPtr->m_credentialsValidated = true;

                    if (sessionPtr->m_readyCallback) {
                        sessionPtr->m_readyCallback();
                    }
                    auto messageBuffer = std::make_shared<std::vector<char>>(1024);
                    socket->async_read_some(boost::asio::buffer(*messageBuffer),
                        [sessionPtr, messageBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                            sessionPtr->handleReadMessage(error, bytes_transferred, messageBuffer);
                        });
                } else {
                    m_clientManager->removeClient(clientId);
                }
            } catch (const std::exception& e) {
                qDebug() << "Ошибка при разборе учетных данных:" << e.what();
                m_clientManager->removeClient(clientId);
            }
        });
}