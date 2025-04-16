#include "async_tcp_server.h"
#include "client_session.h"
#include <QDebug>

AsyncTcpServer::AsyncTcpServer(QObject* parent) : QTcpServer(parent) {
    connect(this, &QTcpServer::newConnection, this, &AsyncTcpServer::onNewConnection);
}

AsyncTcpServer::~AsyncTcpServer() = default;

void AsyncTcpServer::start(uint16_t port) {
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер не смог запуститься! Ошибка:" << errorString();
    } else {
        qDebug() << "Сервер успешно запущен на порту" << port;
    }
}

void AsyncTcpServer::onNewConnection() {
    auto* raw_socket = nextPendingConnection();
    auto session = std::make_unique<ClientSession>(std::unique_ptr<QTcpSocket>(raw_socket));
    QUuid uuid = session->uuid();
    m_clients.emplace(uuid, std::move(session));
    qDebug() << "Новое подключение, UUID:" << uuid;

    connect(m_clients[uuid].get(), &ClientSession::disconnected, 
            this, &AsyncTcpServer::onClientDisconnected);
    connect(m_clients[uuid].get(), &ClientSession::messageReceived,
            this, &AsyncTcpServer::onMessageReceived);
}

void AsyncTcpServer::onClientDisconnected(QUuid uuid) {
    m_clients.erase(uuid);
    qDebug() << "Клиент отключился, UUID:" << uuid;
}

void AsyncTcpServer::onMessageReceived(const std::string& message, QUuid senderId, const std::string& username) {
    qDebug() << "Получено сообщение от" << senderId.toString() 
             << "(" << QString::fromStdString(username) << "):" << QString::fromStdString(message);
    for (const auto& pair : m_clients) {
        if (pair.first != senderId) {
            pair.second->sendMessage(username + ":" + message);
        }
    }
}