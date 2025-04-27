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

    connect(m_socket.get(), &QAbstractSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        qDebug() << "Socket error:" << m_socket->errorString() << "(" << error << ")";
    });
    
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
}

void NetworkClient::onConnected() {
    qDebug() << "Connected to server, sending credentials...";
    
    QString usernameStr = QString::fromStdString(m_username);
    QString passwordStr = QString::fromStdString(m_password); 
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << usernameStr << passwordStr;;

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

        emit messageReceived(msg);

        if (m_messageCallback) {
            m_messageCallback(msg.username, msg.text);
        }
    }

    qint64 processedBytes = stream.device()->pos() - startPos;
    if (processedBytes > 0) {
        m_receivedBuffer.remove(0, processedBytes);
    }
}

void NetworkClient::onDisconnected() {
    qDebug() << "Disconnected from server";
    if (m_disconnectedCallback) {
        m_disconnectedCallback();
    }
    emit disconnected();
    emit connectionStatusChanged(false);
}