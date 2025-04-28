#pragma once
#include "inetwork_client.h"
#include "auth_handler.h"
#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <string>
#include <string_view>
#include <atomic>
#include "message.h"

class NetworkClient : public QObject, public INetworkClient {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() override = default;
    void connectToServer(std::string_view host, std::uint16_t port, 
                        std::string_view username, std::string_view password) override;
    void setConnectionCallback(const ConnectionCallback& callback) override;
    void setDisconnectedCallback(const DisconnectedCallback& callback) override;
    void disconnect() override;
    bool isConnected() const override;

    void sendMessage(std::string_view message) override;
    void requestUserList() override;
    void setMessageCallback(const MessageCallback& callback) override;
    void setUserListCallback(const UserListCallback& callback) override;

signals:
    void userListReceived(const std::vector<QString>& userList);
    void messageReceived(const Message& msg);
    void disconnected();
    void connectionStatusChanged(bool connected);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    void processBuffer();
    void setupSocket();

    std::unique_ptr<QTcpSocket> m_socket;
    std::unique_ptr<IAuthHandler> m_authHandler;
    std::string m_username;
    std::string m_password;
    MessageCallback m_messageCallback;
    ConnectionCallback m_connectionCallback;
    DisconnectedCallback m_disconnectedCallback;
    UserListCallback m_userListCallback;
    QByteArray m_receivedBuffer;
    std::atomic<bool> m_connected;
};