#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>
#include <QAbstractSocket>
#include <QUuid>
#include <QDateTime>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent) {
}

void NetworkClient::connectToServer(std::string_view host, std::uint16_t port, std::string_view username) {
    m_username = username;
    m_socket = std::make_unique<QTcpSocket>(this);

    connect(m_socket.get(), &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, [this]() {
        if (m_disconnectedCallback) {
            m_disconnectedCallback();
        }
        emit disconnected();
    });

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

void NetworkClient::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void NetworkClient::setDisconnectedCallback(const DisconnectedCallback& callback) {
    m_disconnectedCallback = callback;
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
    if (m_messageCallback) {
        m_messageCallback(msg.username, msg.text);
    }
    emit messageReceived(msg.username, msg.text);
}