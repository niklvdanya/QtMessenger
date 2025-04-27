#include "multithreaded_server.h"
#include "boost_asio_client_session.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>

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
    auto session = std::make_unique<BoostAsioClientSession>(socket, m_dbManager);
    QUuid clientId = session->uuid();
    session->setMessageCallback([this](const std::string& message, QUuid senderId, const std::string& username) {
        m_messageHandler->handleMessage(message, senderId, username);
        emit messageReceived(message, senderId, username);
    });

    session->setDisconnectCallback([this](QUuid clientId) {
        m_clientManager->removeClient(clientId);
        emit clientDisconnected(clientId);
        qDebug() << "Client disconnected, UUID:" << clientId;
    });

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

    m_clientManager->addClient(clientId, std::move(session));

    emit newConnection(clientId);
    qDebug() << "New connection, UUID:" << clientId;
}