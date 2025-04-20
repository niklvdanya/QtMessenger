#pragma once
#include <cstdint>
#include <QUuid>
#include <string>

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    virtual void handleMessage(const std::string& message, QUuid senderId, const std::string& username) = 0;
};

class IClientManager {
public:
    virtual ~IClientManager() = default;
    virtual void addClient(QUuid clientId, void* clientData) = 0;
    virtual void removeClient(QUuid clientId) = 0;
    virtual void broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) = 0;
};

class IServer {
public:
    virtual ~IServer() = default;
    virtual void start(uint16_t port) = 0;
    virtual void stop() = 0;
};
