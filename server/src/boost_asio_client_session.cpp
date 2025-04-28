#include "boost_asio_client_session.h"

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>

#include "message.h"

BoostAsioClientSession::BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : m_socket(std::move(socket)),
      m_uuid(QUuid::createUuid()),
      m_username("Guest"),
      m_authenticated(false)
{}

QUuid BoostAsioClientSession::uuid() const noexcept
{
    return m_uuid;
}

std::string BoostAsioClientSession::username() const
{
    return m_username;
}

bool BoostAsioClientSession::isAuthenticated() const
{
    return m_authenticated;
}

void BoostAsioClientSession::sendMessage(const std::string& message)
{
    if (!m_authenticated) {
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

void BoostAsioClientSession::sendMessage(const Message& msg)
{
    if (!m_authenticated && msg.type != MessageType::System) {
        qDebug() << "Client" << m_uuid << "not authenticated. Cannot send message.";
        return;
    }

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << msg;

    auto data = std::make_shared<std::vector<char>>(buffer.begin(), buffer.end());
    auto self = shared_from_this();
    boost::asio::async_write(
        *m_socket, boost::asio::buffer(*data),
        [self, data, msg](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                qDebug() << "Error sending message:" << QString::fromStdString(error.message());
            } else {
                qDebug() << "Sent" << bytes_transferred << "bytes to client" << self->m_uuid
                         << "- Message from:" << QString::fromStdString(msg.username) << "at"
                         << msg.timestamp.toString("hh:mm:ss")
                         << "- Text:" << QString::fromStdString(msg.text);
            }
        });
}

void BoostAsioClientSession::setMessageCallback(const MessageCallback& callback)
{
    m_messageCallback = callback;
}

void BoostAsioClientSession::setDisconnectCallback(const DisconnectCallback& callback)
{
    m_disconnectCallback = callback;
}

std::shared_ptr<boost::asio::ip::tcp::socket> BoostAsioClientSession::getSocket() const
{
    return m_socket;
}

bool BoostAsioClientSession::authenticate(const std::string& username, const std::string& password)
{
    if (!username.empty()) {
        m_username = username;
        m_authenticated = true;
        return true;
    }
    return false;
}

void BoostAsioClientSession::start()
{
    asyncRead();
}

void BoostAsioClientSession::asyncRead()
{
    auto buffer = std::make_shared<std::vector<char>>(4096);
    auto self = shared_from_this();

    m_socket->async_read_some(
        boost::asio::buffer(*buffer),
        [self, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            self->handleReadMessage(error, bytes_transferred, buffer);
        });
}

void BoostAsioClientSession::handleReadMessage(const boost::system::error_code& error,
                                               std::size_t bytes_transferred,
                                               std::shared_ptr<std::vector<char>> buffer)
{
    if (error) {
        qDebug() << "Error reading message:" << QString::fromStdString(error.message());
        if (m_disconnectCallback) {
            m_disconnectCallback(m_uuid);
        }
        return;
    }

    if (!m_authenticated) {
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

        qDebug() << "Message received from" << QString::fromStdString(msg.username) << ":"
                 << QString::fromStdString(msg.text);

        if (m_messageCallback) {
            m_messageCallback(msg, m_uuid);
        }
        asyncRead();
    } catch (const std::exception& e) {
        qDebug() << "Error parsing message from client" << m_uuid << ":" << e.what();
        asyncRead();
    }
}