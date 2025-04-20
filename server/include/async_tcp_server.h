#pragma once
#include <QObject>
#include <QTcpServer>
#include <memory>
#include "iserver.h"
#include "client_manager.h"
#include "message_handler.h"

class AsyncTcpServer : public QTcpServer, public IServer {
    Q_OBJECT
public:
    explicit AsyncTcpServer(QObject* parent = nullptr);
    ~AsyncTcpServer() override;
    void start(uint16_t port) override;
    void stop() override;

private slots:
    void onNewConnection();

private:
    std::shared_ptr<ClientManager> m_clientManager;
    std::unique_ptr<MessageHandler> m_messageHandler;
    bool m_running;
};