#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent) {
}

void NetworkClient::connectToServer(std::string_view host, uint16_t port, std::string_view username) {
    m_username = username;
    m_socket = std::make_unique<QTcpSocket>(this);

    connect(m_socket.get(), &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &NetworkClient::disconnected);

    m_socket->connectToHost(QString::fromStdString(std::string(host)), port);
}

void NetworkClient::sendMessage(std::string_view message) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Не подключен к серверу";
        return;
    }

    Message msg;
    msg.senderId = QUuid::createUuid();
    msg.username = m_username;
    msg.text = std::string(message);
    msg.timestamp = QDateTime::currentDateTime();

    QDataStream stream(m_socket.get());
    stream << msg;
}

void NetworkClient::onConnected() {
    QDataStream stream(m_socket.get());
    stream << QString::fromStdString(m_username);
    qDebug() << "Подключено к серверу, отправлено имя:" << QString::fromStdString(m_username);
}

void NetworkClient::onReadyRead() {
    QDataStream stream(m_socket.get());
    Message msg;
    stream >> msg;
    emit messageReceived(msg.username, msg.text);
}