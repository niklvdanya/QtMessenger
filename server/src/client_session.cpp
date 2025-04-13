#include "client_session.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>

ClientSession::ClientSession(QTcpSocket* socket, QObject* parent) 
    : QObject(parent), m_socket(socket), m_uuid(QUuid::createUuid()), m_username("Guest") {
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientSession::readUsername);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
}

QUuid ClientSession::uuid() const {
    return m_uuid;
}

QString ClientSession::username() const {
    return m_username;
}

void ClientSession::sendMessage(const QString& messageWithSender) {
    int colonPos = messageWithSender.indexOf(':');
    QString username = messageWithSender.left(colonPos);
    QString message = messageWithSender.mid(colonPos + 1);
    
    Message msg;
    msg.senderId = m_uuid;
    msg.username = username;
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();
    
    QDataStream stream(m_socket);
    stream << msg;
    qDebug() << "Отправлено сообщение клиенту" << m_uuid << "(" << m_username << "):" << message;
}

void ClientSession::readUsername() {
    QDataStream stream(m_socket);
    QString username;
    stream >> username;
    if (!username.isEmpty()) {
        m_username = username;
        qDebug() << "Получено имя:" << m_username << "для клиента" << m_uuid;
        disconnect(m_socket, &QTcpSocket::readyRead, this, &ClientSession::readUsername);
        connect(m_socket, &QTcpSocket::readyRead, this, &ClientSession::readMessage);
    }
}

void ClientSession::readMessage() {
    QDataStream stream(m_socket);
    Message msg;
    stream >> msg;
    qDebug() << "Получено сообщение от" << m_uuid << "(" << m_username << "):" << msg.text;
    emit messageReceived(msg.text, m_uuid, m_username); 
}

void ClientSession::onDisconnected() {
    emit disconnected(m_uuid);
    deleteLater();
}