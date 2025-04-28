#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>
#include <QAbstractSocket>
#include <QUuid>
#include <QDateTime>

NetworkClient::NetworkClient(QObject* parent) 
    : QObject(parent)
    , m_connected(false) {
}

void NetworkClient::setupSocket() {
    m_socket = std::make_unique<QTcpSocket>(this);
    m_authHandler = std::make_unique<AuthHandler>(m_socket.get());

    connect(m_socket.get(), &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket.get(), &QAbstractSocket::errorOccurred, this, &NetworkClient::onError);
}

void NetworkClient::connectToServer(std::string_view host, std::uint16_t port, 
                                  std::string_view username, std::string_view password) {
    m_username = username;
    m_password = password;
    
    setupSocket();
    
    qDebug() << "Connecting to server" << QString::fromStdString(std::string(host)) << ":" << port;
    qDebug() << "With username:" << QString::fromStdString(std::string(username));
    
    m_socket->connectToHost(QString::fromStdString(std::string(host)), port);
}

void NetworkClient::disconnect() {
    qDebug() << "Disconnecting from server...";

    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();

        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
        
        qDebug() << "Disconnected from server";
    } else {
        qDebug() << "Not connected to server, no need to disconnect";
    }

    m_receivedBuffer.clear();
    m_connected = false;
}

bool NetworkClient::isConnected() const {
    return m_connected;
}

void NetworkClient::setConnectionCallback(const ConnectionCallback& callback) {
    m_connectionCallback = callback;
}

void NetworkClient::onConnected() {
    qDebug() << "Connected to server, sending credentials...";
    
    m_authHandler->sendCredentials(m_username, m_password);
    
    m_connected = true;
    emit connectionStatusChanged(true);
    
    if (m_connectionCallback) {
        m_connectionCallback(true);
    }
}

void NetworkClient::sendMessage(std::string_view message) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }

    Message msg(QUuid::createUuid(), m_username, std::string(message));

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << msg;
    
    qint64 bytesWritten = m_socket->write(block);
    m_socket->flush();
    
    qDebug() << "Sent message to server (" << bytesWritten << " bytes): " 
             << QString::fromStdString(std::string(message));
}

void NetworkClient::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void NetworkClient::setDisconnectedCallback(const DisconnectedCallback& callback) {
    m_disconnectedCallback = callback;
}

void NetworkClient::setUserListCallback(const UserListCallback& callback) {
    m_userListCallback = callback;
}

void NetworkClient::requestUserList() {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    Message msg(QUuid::createUuid(), m_username, "REQUEST_USER_LIST");
    msg.type = MessageType::System;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << msg;
    
    m_socket->write(block);
    m_socket->flush();
    
    qDebug() << "Sent user list request to server";
}

void NetworkClient::onReadyRead() {
    QByteArray receivedData = m_socket->readAll();
    m_receivedBuffer.append(receivedData);
    processBuffer();
}

void NetworkClient::processBuffer() {
    QDataStream stream(&m_receivedBuffer, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    qint64 startPos = stream.device()->pos();
    while (!stream.atEnd()) {
        qint64 currentPos = stream.device()->pos();
        Message msg;
        stream >> msg;
        if (stream.status() != QDataStream::Ok) {
            stream.device()->seek(currentPos);
            break;
        }
        
        if (msg.type == MessageType::UserList) {
            std::vector<QString> userList;
            userList.reserve(msg.userList.size());
            for (const auto& user : msg.userList) {
                userList.push_back(QString::fromStdString(user));
            }
            
            emit userListReceived(userList);
            
            if (m_userListCallback) {
                m_userListCallback(userList);
            }
        } else {
            emit messageReceived(msg);
            
            if (m_messageCallback) {
                m_messageCallback(msg);
            }
        }
    }
    
    qint64 processedBytes = stream.device()->pos() - startPos;
    if (processedBytes > 0) {
        m_receivedBuffer.remove(0, processedBytes);
    }
}

void NetworkClient::onDisconnected() {
    qDebug() << "Disconnected from server";
    
    m_connected = false;
    
    if (m_disconnectedCallback) {
        m_disconnectedCallback();
    }
    
    emit disconnected();
    emit connectionStatusChanged(false);
    
    if (m_connectionCallback) {
        m_connectionCallback(false);
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError error) {
    qDebug() << "Socket error:" << m_socket->errorString() << "(" << error << ")";
    
    if (m_connectionCallback && m_socket->state() != QAbstractSocket::ConnectedState) {
        m_connectionCallback(false);
    }
}