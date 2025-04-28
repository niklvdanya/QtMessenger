#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <QUuid>

#include "client_session_interface.h"

class IMessageHandler
{
public:
    virtual ~IMessageHandler() = default;
    virtual void handleMessage(const Message& message, QUuid senderId) = 0;
};

class IClientManager
{
public:
    virtual ~IClientManager() = default;
    virtual void addClient(QUuid clientId, std::unique_ptr<IClientSession> client) = 0;
    virtual void removeClient(QUuid clientId) = 0;
    virtual void broadcastMessage(const Message& message) = 0;
    virtual std::vector<std::string> getUsernames() const = 0;
    virtual void sendMessageToClient(QUuid clientId, const Message& message) = 0;
    virtual size_t getClientCount() const = 0;
};

class IServer
{
public:
    virtual ~IServer() = default;
    virtual void start(uint16_t port) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
};