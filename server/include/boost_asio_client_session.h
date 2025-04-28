#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <QUuid>
#include <string>
#include <atomic>
#include "client_session_interface.h"
#include "message.h"
#include "database_manager.h"

class MultithreadedServer;

class BoostAsioClientSession : public IClientSession, 
                              public std::enable_shared_from_this<BoostAsioClientSession> {
public:
    explicit BoostAsioClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    ~BoostAsioClientSession() override = default;
    
    QUuid uuid() const noexcept override;
    std::string username() const override;
    void sendMessage(const std::string& message) override;
    void sendMessage(const Message& msg) override;
    void setMessageCallback(const MessageCallback& callback) override;
    void setDisconnectCallback(const DisconnectCallback& callback) override;
    bool isAuthenticated() const override;
    
    bool authenticate(const std::string& username, const std::string& password);
    void start();
    
    std::shared_ptr<boost::asio::ip::tcp::socket> getSocket() const;

private:
    void handleReadMessage(const boost::system::error_code& error, 
                          std::size_t bytes_transferred, 
                          std::shared_ptr<std::vector<char>> buffer);
    
    void asyncRead();

    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    QUuid m_uuid;
    std::string m_username;
    MessageCallback m_messageCallback;
    DisconnectCallback m_disconnectCallback;
    std::atomic<bool> m_authenticated;
};