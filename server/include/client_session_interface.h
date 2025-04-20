#pragma once
#include <QUuid>
#include <string>
#include <functional>

class IClientSession {
public:
    virtual ~IClientSession() = default;
    
    virtual QUuid uuid() const noexcept = 0;
    virtual std::string username() const = 0;
    virtual void sendMessage(const std::string& message) = 0;
    using MessageCallback = std::function<void(const std::string&, QUuid, const std::string&)>;
    virtual void setMessageCallback(const MessageCallback& callback) = 0;
    
    using DisconnectCallback = std::function<void(QUuid)>;
    virtual void setDisconnectCallback(const DisconnectCallback& callback) = 0;
};