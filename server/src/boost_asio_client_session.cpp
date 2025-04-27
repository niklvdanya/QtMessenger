#include "boost_asio_client_session.h"
#include "message.h"
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QIODevice>

BoostAsioClientSession::BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, DatabaseManager* dbManager)
    : m_socket(socket), m_uuid(QUuid::createUuid()), m_username("Guest"), m_dbManager(dbManager) {
}

QUuid BoostAsioClientSession::uuid() const noexcept {
    return m_uuid;
}

std::string BoostAsioClientSession::username() const {
    return m_username;
}

void BoostAsioClientSession::sendMessage(const std::string& message) {
    if (!m_credentialsValidated) {
        qDebug() << "Client" << m_uuid << "not authenticated. Cannot send message.";
        return;
    }

    Message msg;
    msg.senderId = m_uuid;
    msg.username = m_username;
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();

    sendMessage(msg);
}

void BoostAsioClientSession::sendMessage(const Message& msg) {
    if (!m_credentialsValidated) {
        qDebug() << "Client" << m_uuid << "not authenticated. Cannot send message.";
        return;
    }

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << msg;

    auto data = std::make_shared<std::vector<char>>(buffer.begin(), buffer.end());
    boost::asio::async_write(*m_socket, boost::asio::buffer(*data),
        [this, data](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                qDebug() << "Error sending message:" << QString::fromStdString(error.message());
            } else {
                qDebug() << "Sent" << bytes_transferred << "bytes to client" << m_uuid;
            }
        });
}

void BoostAsioClientSession::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void BoostAsioClientSession::setDisconnectCallback(const DisconnectCallback& callback) {
    m_disconnectCallback = callback;
}

void BoostAsioClientSession::setReadyCallback(const ReadyCallback& callback) {
    m_readyCallback = callback;
}

std::shared_ptr<boost::asio::ip::tcp::socket> BoostAsioClientSession::getSocket() const {
    return m_socket;
}

void BoostAsioClientSession::handleReadMessage(const boost::system::error_code& error, 
                                             std::size_t bytes_transferred, 
                                             std::shared_ptr<std::vector<char>> buffer) {
    if (error) {
        qDebug() << "Error reading message:" << QString::fromStdString(error.message());
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
        return;
    }
    if (!m_credentialsValidated) {
        qDebug() << "Received message from non-authenticated client" << m_uuid;
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
        return;
    }
    try {
        QByteArray data(buffer->data(), static_cast<int>(bytes_transferred));
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_0);
        
        Message msg;
        stream >> msg;

        qDebug() << "Message received from" << QString::fromStdString(msg.username) 
                 << ":" << QString::fromStdString(msg.text);

        if (m_messageCallback) {
            m_messageCallback(msg.text, m_uuid, m_username);
        }
        auto newBuffer = std::make_shared<std::vector<char>>(4096); 
        m_socket->async_read_some(boost::asio::buffer(*newBuffer),
            [this, newBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReadMessage(error, bytes_transferred, newBuffer);
            });
    } catch (const std::exception& e) {
        qDebug() << "Error parsing message from client" << m_uuid << ":" << e.what();
        auto newBuffer = std::make_shared<std::vector<char>>(4096);
        m_socket->async_read_some(boost::asio::buffer(*newBuffer),
            [this, newBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReadMessage(error, bytes_transferred, newBuffer);
            });
    }
}