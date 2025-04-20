#include "boost_asio_client_session.h"
#include <QDebug>

BoostAsioClientSession::BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : m_socket(socket), m_uuid(QUuid::createUuid()), m_username("Guest") {}

QUuid BoostAsioClientSession::uuid() const noexcept {
    return m_uuid;
}

std::string BoostAsioClientSession::username() const {
    return m_username;
}

void BoostAsioClientSession::sendMessage(const std::string& message) {
    boost::asio::async_write(*m_socket, boost::asio::buffer(message),
        [](const boost::system::error_code&, std::size_t) {});
}

void BoostAsioClientSession::setMessageCallback(const MessageCallback& callback) {
    m_messageCallback = callback;
}

void BoostAsioClientSession::setDisconnectCallback(const DisconnectCallback& callback) {
    m_disconnectCallback = callback;
}

std::shared_ptr<boost::asio::ip::tcp::socket> BoostAsioClientSession::getSocket() const {
    return m_socket;
}