#pragma once
#include <QObject>
#include <QTcpServer>
#include <unordered_map>
#include <memory>
#include <QUuid>
#include "quuid_hash.h"

class ClientSession;

class AsyncTcpServer : public QTcpServer {
    Q_OBJECT
public:
    explicit AsyncTcpServer(QObject* parent = nullptr);
    ~AsyncTcpServer() override;

    void start(uint16_t port);

private slots:
    void onNewConnection();
    void onClientDisconnected(QUuid uuid);
    void onMessageReceived(const std::string& message, QUuid senderId, const std::string& username);

private:
    std::unordered_map<QUuid, std::unique_ptr<ClientSession>> m_clients;
};