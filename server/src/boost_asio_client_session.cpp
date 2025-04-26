#include "boost_asio_client_session.h"
#include "message.h"
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QIODevice>

BoostAsioClientSession::BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : m_socket(socket), m_uuid(QUuid::createUuid()), m_username("Guest") {
    readUsername();
}

QUuid BoostAsioClientSession::uuid() const noexcept {
    return m_uuid;
}

std::string BoostAsioClientSession::username() const {
    return m_username;
}

void BoostAsioClientSession::sendMessage(const std::string& message) {
    auto colonPos = message.find(':');
    if (colonPos == std::string::npos) {
        qDebug() << "Invalid message format:" << QString::fromStdString(message);
        return;
    }
    std::string username = message.substr(0, colonPos);
    std::string text = message.substr(colonPos + 1);

    Message msg;
    msg.senderId = m_uuid;
    msg.username = username;
    msg.text = text;
    msg.timestamp = QDateTime::currentDateTime();

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << msg;

    auto data = std::make_shared<std::vector<char>>(buffer.begin(), buffer.end());
    boost::asio::async_write(*m_socket, boost::asio::buffer(*data),
        [data](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            if (error) {
                qDebug() << "Error sending message:" << QString::fromStdString(error.message());
            }
        });
}

void BoostAsioClientSession::sendMessage(const Message& msg) {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << msg;

    auto data = std::make_shared<std::vector<char>>(buffer.begin(), buffer.end());
    boost::asio::async_write(*m_socket, boost::asio::buffer(*data),
        [data](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            if (error) {
                qDebug() << "Error sending message:" << QString::fromStdString(error.message());
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

void BoostAsioClientSession::readUsername() {
    auto buffer = std::make_shared<std::vector<char>>(1024);
    m_socket->async_read_some(boost::asio::buffer(*buffer),
        [this, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handleReadUsername(error, bytes_transferred, buffer);
        });
}

void BoostAsioClientSession::handleReadUsername(const boost::system::error_code& error, 
                                               std::size_t bytes_transferred, 
                                               std::shared_ptr<std::vector<char>> buffer) {
    if (error) {
        qDebug() << "Error reading username:" << QString::fromStdString(error.message());
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
        return;
    }

    QByteArray data(buffer->data(), static_cast<int>(bytes_transferred));
    QDataStream stream(data);
    QString username;
    stream >> username;

    if (!username.isEmpty()) {
        m_username = username.toStdString();
        m_usernameRead = true;
        qDebug() << "Received username:" << QString::fromStdString(m_username) 
                 << "for client" << m_uuid;

        if (m_readyCallback) {
            m_readyCallback();
        }

        auto newBuffer = std::make_shared<std::vector<char>>(1024);
        m_socket->async_read_some(boost::asio::buffer(*newBuffer),
            [this, newBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReadMessage(error, bytes_transferred, newBuffer);
            });
    }
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

    QByteArray data(buffer->data(), static_cast<int>(bytes_transferred));
    QDataStream stream(data);
    Message msg;
    stream >> msg;

    if (m_messageCallback) {
        m_messageCallback(msg.text, m_uuid, msg.username);
    }

    auto newBuffer = std::make_shared<std::vector<char>>(1024);
    m_socket->async_read_some(boost::asio::buffer(*newBuffer),
        [this, newBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handleReadMessage(error, bytes_transferred, newBuffer);
        });
}