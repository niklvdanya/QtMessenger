#include "async_tcp_server.h"
#include "client_session.h"
#include <QDebug>

AsyncTcpServer::AsyncTcpServer(QObject* parent) 
    : QTcpServer(parent), m_pool(), m_clients() {
    connect(this, &QTcpServer::newConnection, this, &AsyncTcpServer::onNewConnection);
}

AsyncTcpServer::~AsyncTcpServer() {
    m_clients.clear();
}

void AsyncTcpServer::start(quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер не смог запуститься! Ошибка:" << errorString();
    } else {
        qDebug() << "Сервер успешно запущен на порту" << port;
    }
}

void AsyncTcpServer::onNewConnection() {
    QTcpSocket* socket = nextPendingConnection();
    auto* session = new ClientSession(socket);
    m_clients.emplace(session->uuid(), std::unique_ptr<ClientSession>(session));
    qDebug() << "Новое подключение, UUID:" << session->uuid();
    
    connect(session, &ClientSession::disconnected, 
            this, &AsyncTcpServer::onClientDisconnected);
    connect(session, &ClientSession::messageReceived,
            this, &AsyncTcpServer::onMessageReceived);
}

void AsyncTcpServer::onClientDisconnected(QUuid uuid) {
    m_clients.erase(uuid);
    qDebug() << "Клиент отключился, UUID:" << uuid;
}

void AsyncTcpServer::onMessageReceived(const QString& message, QUuid senderId, const QString& username) {
    qDebug() << "Получено сообщение от" << senderId << "(" << username << "):" << message;
    for (const auto& pair : m_clients) {
        if (pair.first != senderId) {
            pair.second->sendMessage(username + ":" + message);
        }
    }
}