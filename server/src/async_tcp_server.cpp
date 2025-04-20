#include "async_tcp_server.h"
#include "client_session.h"
#include <QDebug>

AsyncTcpServer::AsyncTcpServer(QObject* parent) 
    : QTcpServer(parent), 
      m_clientManager(std::make_shared<ClientManager>()),
      m_messageHandler(std::make_unique<MessageHandler>(m_clientManager)),
      m_running(false) {
    connect(this, &QTcpServer::newConnection, this, &AsyncTcpServer::onNewConnection);
}

AsyncTcpServer::~AsyncTcpServer() {
    stop();
}

void AsyncTcpServer::start(uint16_t port) {
    if (m_running) return;
    
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Server failed to start! Error:" << errorString();
    } else {
        m_running = true;
        qDebug() << "Server started on port" << port;
    }
}

void AsyncTcpServer::stop() {
    if (!m_running) return;
    
    m_running = false;
    close();
    m_clientManager->clear();
    qDebug() << "Server stopped";
}

void AsyncTcpServer::onNewConnection() {
    auto* raw_socket = nextPendingConnection();
    auto session = std::make_unique<ClientSession>(std::unique_ptr<QTcpSocket>(raw_socket));
    QUuid uuid = session->uuid();
    
    session->setMessageCallback([this](const std::string& message, QUuid senderId, const std::string& username) {
        m_messageHandler->handleMessage(message, senderId, username);
    });
    
    session->setDisconnectCallback([this](QUuid clientId) {
        m_clientManager->removeClient(clientId);
    });
    
    m_clientManager->addClient(uuid, std::move(session));
    
    qDebug() << "New connection, UUID:" << uuid;
}