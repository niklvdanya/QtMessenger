#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <QUuid>
#include <string>
#include "client_session_interface.h"
#include "message.h"

class BoostAsioClientSession : public IClientSession {
public:
    explicit BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    QUuid uuid() const noexcept override;
    std::string username() const override;
    void sendMessage(const std::string& message) override;
    void sendMessage(const Message& msg);
    void setMessageCallback(const MessageCallback& callback) override;
    void setDisconnectCallback(const DisconnectCallback& callback) override;
    std::shared_ptr<boost::asio::ip::tcp::socket> getSocket() const;

    void readUsername();

    using ReadyCallback = std::function<void()>;
    void setReadyCallback(const ReadyCallback& callback);

private:
    void handleReadUsername(const boost::system::error_code& error, std::size_t bytes_transferred, 
                           std::shared_ptr<std::vector<char>> buffer);
    void handleReadMessage(const boost::system::error_code& error, std::size_t bytes_transferred, 
                          std::shared_ptr<std::vector<char>> buffer);

    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    QUuid m_uuid;
    std::string m_username;
    MessageCallback m_messageCallback;
    DisconnectCallback m_disconnectCallback;
    ReadyCallback m_readyCallback;
    bool m_usernameRead{false};
};