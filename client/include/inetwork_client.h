#pragma once
#include <cstdint>
#include <functional>
#include <string_view>

class IMessageReceiver {
public:
    virtual ~IMessageReceiver() = default;
    
    using MessageCallback = std::function<void(const std::string&, const std::string&)>;
    virtual void setMessageCallback(const MessageCallback& callback) = 0;
};

class IConnectionHandler {
public:
    virtual ~IConnectionHandler() = default;
    
    using DisconnectedCallback = std::function<void()>;
    virtual void setDisconnectedCallback(const DisconnectedCallback& callback) = 0;
    virtual void connectToServer(std::string_view host, std::uint16_t port, 
                                 std::string_view username, std::string_view password) = 0;
    virtual void disconnect() = 0; 
};

class IMessageSender {
public:
    virtual ~IMessageSender() = default;
    virtual void sendMessage(std::string_view message) = 0;
};

class INetworkClient : public IMessageReceiver, public IConnectionHandler, public IMessageSender {
public:
    ~INetworkClient() override = default;
};