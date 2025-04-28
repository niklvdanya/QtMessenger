#pragma once
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

#include "message.h"

class IMessageReceiver
{
public:
    virtual ~IMessageReceiver() = default;

    using MessageCallback = std::function<void(const Message&)>;
    virtual void setMessageCallback(const MessageCallback& callback) = 0;
};

class IConnectionHandler
{
public:
    virtual ~IConnectionHandler() = default;

    using ConnectionCallback = std::function<void(bool)>;
    using DisconnectedCallback = std::function<void()>;

    virtual void setConnectionCallback(const ConnectionCallback& callback) = 0;
    virtual void setDisconnectedCallback(const DisconnectedCallback& callback) = 0;
    virtual void connectToServer(std::string_view host, std::uint16_t port,
                                 std::string_view username, std::string_view password) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

class IMessageSender
{
public:
    virtual ~IMessageSender() = default;
    virtual void sendMessage(std::string_view message) = 0;
    virtual void requestUserList() = 0;
};

class INetworkClient : public IMessageReceiver, public IConnectionHandler, public IMessageSender
{
public:
    ~INetworkClient() override = default;

    using UserListCallback = std::function<void(const std::vector<QString>&)>;
    virtual void setUserListCallback(const UserListCallback& callback) = 0;
};