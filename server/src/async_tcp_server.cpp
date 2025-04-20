#include "async_tcp_server.h"
#include "client_session.h"
#include <QDebug>

AsyncTcpServer::AsyncTcpServer(QObject* parent) 
    : QTcpServer(parent), m_running(false) {
    connect(this, &QTcpServer::newConnection, this, &AsyncTcpServer::onNewConnection);
}

AsyncTcpServer::~AsyncTcpServer() {
    stop();
}

void AsyncTcpServer::start(uint16_t port) {
    if (m_running) return;
    
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер не смог запуститься! Ошибка:" << errorString();
    } else {
        m_running = true;
        qDebug() << "Сервер успешно запущен на порту" << port;
    }
}

void AsyncTcpServer::stop() {
    if (!m_running) return;
    
    m_running = false;
    close();
    m_clients.clear();
    qDebug() << "Сервер остановлен";
}

void AsyncTcpServer::addClient(QUuid clientId, void* clientData) {
}

void AsyncTcpServer::removeClient(QUuid clientId) {
    m_clients.erase(clientId);
    qDebug() << "Клиент удален, UUID:" << clientId;
}

void AsyncTcpServer::broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) {
    for (const auto& pair : m_clients) {
        if (pair.first != senderId) {
            pair.second->sendMessage(username + ":" + message);
        }
    }
}

void AsyncTcpServer::handleMessage(const std::string& message, QUuid senderId, const std::string& username) {
    qDebug() << "Обработка сообщения от" << senderId.toString() 
             << "(" << QString::fromStdString(username) << "):" << QString::fromStdString(message);
    broadcastMessage(message, senderId, username);
}

void AsyncTcpServer::onNewConnection() {
    auto* raw_socket = nextPendingConnection();
    auto session = std::make_unique<ClientSession>(std::unique_ptr<QTcpSocket>(raw_socket));
    QUuid uuid = session->uuid();
    
    session->setMessageCallback([this](const std::string& message, QUuid senderId, const std::string& username) {
        this->handleMessage(message, senderId, username);
    });
    
    session->setDisconnectCallback([this](QUuid clientId) {
        this->removeClient(clientId);
    });
    
    m_clients.emplace(uuid, std::move(session));
    qDebug() << "Новое подключение, UUID:" << uuid;
    
    connect(dynamic_cast<ClientSession*>(m_clients[uuid].get()), &ClientSession::disconnected, 
            this, &AsyncTcpServer::onClientDisconnected);
    connect(dynamic_cast<ClientSession*>(m_clients[uuid].get()), &ClientSession::messageReceived,
            this, &AsyncTcpServer::onMessageReceived);
}

void AsyncTcpServer::onClientDisconnected(QUuid uuid) {
    removeClient(uuid);
}

void AsyncTcpServer::onMessageReceived(const std::string& message, QUuid senderId, const std::string& username) {
    handleMessage(message, senderId, username);
}