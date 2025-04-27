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

class SharedSessionWrapper : public IClientSession {
public:
    explicit SharedSessionWrapper(std::shared_ptr<BoostAsioClientSession> session) : m_session(session) {}
    
    QUuid uuid() const noexcept override { return m_session->uuid(); }
    std::string username() const override { return m_session->username(); }
    void sendMessage(const std::string& message) override { m_session->sendMessage(message); }
    void sendMessage(const Message& msg) override { m_session->sendMessage(msg); }
    void setMessageCallback(const MessageCallback& callback) override { m_session->setMessageCallback(callback); }
    void setDisconnectCallback(const DisconnectCallback& callback) override { m_session->setDisconnectCallback(callback); }
    
private:
    std::shared_ptr<BoostAsioClientSession> m_session;
};

void MultithreadedServer::handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto session = std::make_shared<BoostAsioClientSession>(socket, nullptr);
    QUuid clientId = session->uuid();
    session->setMessageCallback([this](const std::string& message, QUuid senderId, const std::string& username) {
        m_messageHandler->handleMessage(message, senderId, username);
        emit messageReceived(message, senderId, username);
    });

    session->setDisconnectCallback([this, clientId](QUuid) {
        m_clientManager->removeClient(clientId);
        emit clientDisconnected(clientId);
        qDebug() << "Client disconnected, UUID:" << clientId;
    });
    auto buffer = std::make_shared<std::vector<char>>(1024);
    socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, socket, buffer, clientId, session](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                qDebug() << "Error reading credentials:" << QString::fromStdString(error.message());
                return;
            }
            
            try {
                QByteArray rawData(buffer->data(), static_cast<int>(bytes_transferred));
                QDataStream stream(&rawData, QIODevice::ReadOnly);
                stream.setVersion(QDataStream::Qt_6_0);
                
                QString username, password;
                stream >> username >> password;
                bool validCredentials = acceptAllCredentials(username, password);
                
                if (validCredentials) {
                    qDebug() << "New User:" << username;
                    session->m_username = username.toStdString();
                    session->m_credentialsValidated = true;
                    
                    auto wrapper = std::make_unique<SharedSessionWrapper>(session);
                    m_clientManager->addClient(clientId, std::move(wrapper));
                    emit newConnection(clientId);
                    qDebug() << "New connection added to client manager, UUID:" << clientId;
                    
                    auto messageBuffer = std::make_shared<std::vector<char>>(4096);
                    socket->async_read_some(boost::asio::buffer(*messageBuffer),
                        [session, messageBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                            session->handleReadMessage(error, bytes_transferred, messageBuffer);
                        });
                } else {
                    qDebug() << "Wrong data for: " << username;
                }
            } catch (const std::exception& e) {
                qDebug() << "Error:" << e.what();
            }
        });
}