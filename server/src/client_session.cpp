#include "client_session.h"
#include "message.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QDebug>

ClientSession::ClientSession(std::unique_ptr<QTcpSocket> socket, QObject* parent) 
    : QObject(parent), m_socket(std::move(socket)), m_uuid(QUuid::createUuid()), m_username("Guest") {
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &ClientSession::readUsername);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
}

QUuid ClientSession::uuid() const noexcept {
    return m_uuid;
}

std::string ClientSession::username() const {
    return m_username;
}

void ClientSession::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void ClientSession::setDisconnectCallback(const DisconnectCallback& callback) {
    m_disconnectCallback = callback;
}

void ClientSession::sendMessage(const std::string& messageWithSender) {
    auto colonPos = messageWithSender.find(':');
    std::string username = messageWithSender.substr(0, colonPos);
    std::string message = messageWithSender.substr(colonPos + 1);

    Message msg;
    msg.senderId = m_uuid;
    msg.username = username;
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();

    QDataStream stream(m_socket.get());
    stream << msg;
    qDebug() << "Отправлено сообщение клиенту" << m_uuid << "(" 
             << QString::fromStdString(m_username) << "):" << QString::fromStdString(message);
}

void ClientSession::readUsername() {
    QDataStream stream(m_socket.get());
    QString username;
    stream >> username;
    if (!username.isEmpty()) {
        m_username = username.toStdString();
        qDebug() << "Получено имя:" << QString::fromStdString(m_username) 
                 << "для клиента" << m_uuid;
        disconnect(m_socket.get(), &QTcpSocket::readyRead, this, &ClientSession::readUsername);
        connect(m_socket.get(), &QTcpSocket::readyRead, this, &ClientSession::readMessage);
    }
}

void ClientSession::readMessage() {
    QDataStream stream(m_socket.get());
    Message msg;
    stream >> msg;
    qDebug() << "Получено сообщение от" << m_uuid << "(" 
             << QString::fromStdString(m_username) << "):" << QString::fromStdString(msg.text);
    
    if (m_messageCallback) {
        m_messageCallback(msg.text, m_uuid, m_username);
    }
    
    emit messageReceived(msg.text, m_uuid, m_username);
}

void ClientSession::onDisconnected() {
    if (m_disconnectCallback) {
        m_disconnectCallback(m_uuid);
    }
    
    emit disconnected(m_uuid);
    deleteLater();
}