#include "client_session.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>

ClientSession::ClientSession(QTcpSocket* socket, QObject* parent) 
    : QObject(parent), m_socket(socket), m_uuid(QUuid::createUuid()) {
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientSession::readMessage);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
}

QUuid ClientSession::uuid() const {
    return m_uuid;
}

void ClientSession::sendMessage(const QString& message) {
    Message msg;
    msg.senderId = m_uuid;
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();
    QDataStream stream(m_socket);
    stream << msg;
    qDebug() << "Отправлено сообщение клиенту" << m_uuid << ":" << message;
}

void ClientSession::readMessage() {
    QDataStream stream(m_socket);
    Message msg;
    stream >> msg;
    qDebug() << "Получено сообщение от" << m_uuid << ":" << msg.text;
    emit messageReceived(msg.text, m_uuid);
}

void ClientSession::onDisconnected() {
    emit disconnected(m_uuid);
    deleteLater();
}