#pragma once
#include <QObject>
#include <QTcpServer>
#include <unordered_map>
#include <memory>
#include <QUuid>
#include "quuid_hash.h"
#include "iserver.h"
#include "client_session_interface.h"

class ClientSession;

class AsyncTcpServer : public QTcpServer, public IServer, public IClientManager, public IMessageHandler {
    Q_OBJECT
public:
    explicit AsyncTcpServer(QObject* parent = nullptr);
    ~AsyncTcpServer() override;
    void start(uint16_t port) override;
    void stop() override;
    void addClient(QUuid clientId, void* clientData) override;
    void removeClient(QUuid clientId) override;
    void broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) override;
    
    void handleMessage(const std::string& message, QUuid senderId, const std::string& username) override;

private slots:
    void onNewConnection();
    void onClientDisconnected(QUuid uuid);
    void onMessageReceived(const std::string& message, QUuid senderId, const std::string& username);

private:
    std::unordered_map<QUuid, std::unique_ptr<IClientSession>> m_clients;
    bool m_running;
};
