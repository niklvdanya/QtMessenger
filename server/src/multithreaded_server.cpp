#include "multithreaded_server.h"

#include <functional>

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <QSqlDatabase>
#include <QThread>

#include "boost_asio_client_session.h"

class SharedSessionWrapper : public IClientSession
{
public:
    explicit SharedSessionWrapper(std::shared_ptr<BoostAsioClientSession> session)
        : m_session(std::move(session))
    {}

    QUuid uuid() const noexcept override { return m_session->uuid(); }
    std::string username() const override { return m_session->username(); }
    void sendMessage(const std::string& message) override { m_session->sendMessage(message); }
    void sendMessage(const Message& msg) override { m_session->sendMessage(msg); }
    void setMessageCallback(const MessageCallback& callback) override
    {
        m_session->setMessageCallback(callback);
    }
    void setDisconnectCallback(const DisconnectCallback& callback) override
    {
        m_session->setDisconnectCallback(callback);
    }
    bool isAuthenticated() const override { return m_session->isAuthenticated(); }

private:
    std::shared_ptr<BoostAsioClientSession> m_session;
};

MultithreadedServer::MultithreadedServer(unsigned short port, int thread_count,
                                         IDatabase* dbManager, QObject* parent)
    : QObject(parent),
      m_io_context(),
      m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      m_dbManager(dbManager),
      m_clientManager(std::make_shared<ClientManager>()),
      m_messageHandler(std::make_shared<MessageHandler>(m_clientManager)),
      m_running(false)
{
    m_threads.reserve(thread_count);
}

MultithreadedServer::~MultithreadedServer()
{
    stop();
}

void MultithreadedServer::start(uint16_t port)
{
    if (m_running)
        return;

    m_running = true;
    accept_connections();

    for (size_t i = 0; i < m_threads.capacity(); ++i) {
        m_threads.emplace_back([this]() { m_io_context.run(); });
    }

    qDebug() << "Multithreaded server started on port" << m_acceptor.local_endpoint().port()
             << "with" << m_threads.size() << "worker threads";
}

void MultithreadedServer::stop()
{
    if (!m_running)
        return;

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

bool MultithreadedServer::isRunning() const
{
    return m_running;
}

void MultithreadedServer::accept_connections()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);

    m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            handle_connection(socket);
        } else {
            qDebug() << "Error accepting connection:" << QString::fromStdString(error.message());
        }

        if (m_running) {
            accept_connections();
        }
    });
}

void MultithreadedServer::handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    qDebug() << "New connection from:"
             << QString::fromStdString(socket->remote_endpoint().address().to_string());

    auto buffer = std::make_shared<std::vector<char>>(1024);

    socket->async_read_some(
        boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& error,
                                                             std::size_t bytes_transferred) {
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

                auto session = std::make_shared<BoostAsioClientSession>(socket);
                QUuid clientId = session->uuid();
                bool authenticated =
                    authenticate_client(username.toStdString(), password.toStdString());

                if (authenticated) {
                    qDebug() << "User authenticated:" << username;
                    session->authenticate(username.toStdString(), password.toStdString());
                    session->setMessageCallback(
                        [this, clientId](const Message& message, QUuid senderId) {
                            m_messageHandler->handleMessage(message, senderId);
                            emit messageReceived(message);
                        });

                    session->setDisconnectCallback([this, clientId](QUuid) {
                        m_clientManager->removeClient(clientId);
                        emit clientDisconnected(clientId);
                        qDebug() << "Client disconnected, UUID:" << clientId;
                    });

                    auto wrapper = std::make_unique<SharedSessionWrapper>(session);
                    m_clientManager->addClient(clientId, std::move(wrapper));

                    session->start();

                    emit newConnection(clientId);
                    qDebug() << "New authenticated connection added, UUID:" << clientId;
                } else {
                    qDebug() << "Authentication failed for:" << username;
                    socket->close();
                }
            } catch (const std::exception& e) {
                qDebug() << "Error processing credentials:" << e.what();
                socket->close();
            }
        });
}

bool MultithreadedServer::authenticate_client(const std::string& username,
                                              const std::string& password)
{
    if (QThread::currentThread() != thread()) {
        bool result = false;
        QMetaObject::invokeMethod(
            this,
            [this, &username, &password, &result]() {
                result = this->authenticate_client_impl(username, password);
            },
            Qt::BlockingQueuedConnection);
        return result;
    }

    return authenticate_client_impl(username, password);
}

bool MultithreadedServer::authenticate_client_impl(const std::string& username,
                                                   const std::string& password)
{
    if (m_dbManager) {
        return m_dbManager->checkUser(QString::fromStdString(username),
                                      QString::fromStdString(password));
    }
    return !username.empty() && !password.empty();
}