#include "boost_asio_client_session.h"
#include "message.h"
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QIODevice>

BoostAsioClientSession::BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, DatabaseManager* dbManager)
    : m_socket(socket), m_uuid(QUuid::createUuid()), m_username("Guest"), m_dbManager(dbManager) {
    readCredentials();
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
    if (!m_credentialsValidated) {
        qDebug() << "Client" << m_uuid << "not authenticated. Cannot send message.";
        return;
    }

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

void BoostAsioClientSession::readCredentials() {
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
        qDebug() << "Received username:" << QString::fromStdString(m_username) 
                 << "for client" << m_uuid;

        // Читаем пароль
        auto passwordBuffer = std::make_shared<std::vector<char>>(1024);
        m_socket->async_read_some(boost::asio::buffer(*passwordBuffer),
            [this, passwordBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReadPassword(error, bytes_transferred, passwordBuffer);
            });
    } else {
        qDebug() << "Empty username received for client" << m_uuid;
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
    }
}

void BoostAsioClientSession::handleReadPassword(const boost::system::error_code& error, 
                                               std::size_t bytes_transferred, 
                                               std::shared_ptr<std::vector<char>> buffer) {
    if (error) {
        qDebug() << "Error reading password:" << QString::fromStdString(error.message());
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
        return;
    }

    QByteArray data(buffer->data(), static_cast<int>(bytes_transferred));
    QDataStream stream(data);
    QString password;
    stream >> password;

    if (!password.isEmpty()) {
        m_password = password.toStdString();
        qDebug() << "Received password for client" << m_uuid;

        // Проверяем учетные данные
        if (m_dbManager->checkUser(QString::fromStdString(m_username), password)) {
            m_credentialsValidated = true;
            qDebug() << "Client" << m_uuid << "authenticated successfully as" << QString::fromStdString(m_username);

            if (m_readyCallback) {
                m_readyCallback();
            }

            // Начинаем чтение сообщений
            auto messageBuffer = std::make_shared<std::vector<char>>(1024);
            m_socket->async_read_some(boost::asio::buffer(*messageBuffer),
                [this, messageBuffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    handleReadMessage(error, bytes_transferred, messageBuffer);
                });
        } else {
            qDebug() << "Invalid credentials for client" << m_uuid;
            if (m_disconnectCallback) {
                m_disconnectCallback(m_uuid);
            }
        }
    } else {
        qDebug() << "Empty password received for client" << m_uuid;
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
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