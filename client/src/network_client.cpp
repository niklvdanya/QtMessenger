#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>
#include <QAbstractSocket>
#include <QUuid>
#include <QDateTime>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent) {
}

void NetworkClient::connectToServer(std::string_view host, std::uint16_t port, std::string_view username, std::string_view password) {
    m_username = username;
    m_password = password;
    m_socket = std::make_unique<QTcpSocket>(this);
    m_authHandler = std::make_unique<AuthHandler>(m_socket.get());

    connect(m_socket.get(), &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    
    qDebug() << "Connecting to server" << QString::fromStdString(std::string(host)) << ":" << port;
    qDebug() << "With username:" << QString::fromStdString(std::string(username));
    qDebug() << "Password length:" << password.length();
    
    m_socket->connectToHost(QString::fromStdString(std::string(host)), port);
}

void NetworkClient::onConnected() {
    qDebug() << "Connected to server, sending credentials...";
    
    // Отправляем оба значения в одном потоке данных
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    
    // Отправляем имя пользователя и пароль в одном сообщении
    out << QString::fromStdString(m_username) << QString::fromStdString(m_password);
    
    qDebug() << "Sending credentials - username:" << QString::fromStdString(m_username) 
             << ", password length:" << m_password.length();
    
    m_socket->write(block);
    m_socket->flush();
    
    emit connectionStatusChanged(true);
}

void NetworkClient::sendMessage(std::string_view message) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }

    Message msg;
    msg.senderId = QUuid::createUuid();
    msg.username = m_username;
    msg.text = std::string(message);
    msg.timestamp = QDateTime::currentDateTime();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << msg;
    
    m_socket->write(block);
    m_socket->flush();
    
    qDebug() << "Sent message to server:" << QString::fromStdString(std::string(message));
}

void NetworkClient::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void NetworkClient::setDisconnectedCallback(const DisconnectedCallback& callback) {
    m_disconnectedCallback = callback;
}

void NetworkClient::onReadyRead() {
    QDataStream in(m_socket.get());
    in.setVersion(QDataStream::Qt_6_0);
    
    Message msg;
    in >> msg;
    
    qDebug() << "Received message from server, username:" 
             << QString::fromStdString(msg.username) 
             << ", text:" << QString::fromStdString(msg.text);
             
    if (m_messageCallback) {
        m_messageCallback(msg.username, msg.text);
    }
    emit messageReceived(msg);
}

void NetworkClient::onDisconnected() {
    qDebug() << "Disconnected from server";
    if (m_disconnectedCallback) {
        m_disconnectedCallback();
    }
    emit disconnected();
    emit connectionStatusChanged(false);
}