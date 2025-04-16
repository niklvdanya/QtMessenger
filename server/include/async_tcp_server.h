#pragma once
#include <QTcpServer>
#include <QThreadPool>
#include <unordered_map>
#include <memory>
#include <QObject>
#include <QUuid>
#include <QHash>
#include "quuid_hash.h"

class ClientSession;

class AsyncTcpServer : public QTcpServer {
    Q_OBJECT
public:
    explicit AsyncTcpServer(QObject* parent = nullptr);
    ~AsyncTcpServer();
    void start(quint16 port);

private slots:
    void onNewConnection();
    void onClientDisconnected(QUuid uuid);
    void onMessageReceived(const QString& message, QUuid senderId, const QString& username);

private:
    QThreadPool m_pool;
    std::unordered_map<QUuid, std::unique_ptr<ClientSession>> m_clients;
};