#pragma once
#include "inetwork_client.h"
#include "auth_handler.h"
#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <string>
#include <string_view>
#include "message.h"

class AuthHandler;

class NetworkClient : public QObject, public INetworkClient {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() override = default;

    void connectToServer(std::string_view host, std::uint16_t port, 
                         std::string_view username, std::string_view password) override;
    void setDisconnectedCallback(const DisconnectedCallback& callback) override;
    void sendMessage(std::string_view message) override;
    void setMessageCallback(const MessageCallback& callback) override;

signals:
    void messageReceived(const Message& msg);
    void disconnected();
    void connectionStatusChanged(bool connected);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected(); 

private:
    void processBuffer();

    std::unique_ptr<QTcpSocket> m_socket;
    std::unique_ptr<AuthHandler> m_authHandler;
    std::string m_username;
    std::string m_password;
    MessageCallback m_messageCallback;
    DisconnectedCallback m_disconnectedCallback;
    QByteArray m_receivedBuffer;
};