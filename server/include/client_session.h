#pragma once
#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <QUuid>
#include <string>
#include "client_session_interface.h"

class ClientSession : public QObject, public IClientSession {
    Q_OBJECT
public:
    explicit ClientSession(std::unique_ptr<QTcpSocket> socket, QObject* parent = nullptr);
    ~ClientSession() override = default;
    QUuid uuid() const noexcept override;
    std::string username() const override;
    void sendMessage(const std::string& message) override;
    void sendMessage(const Message& msg) override; 
    void setMessageCallback(const MessageCallback& callback) override;
    void setDisconnectCallback(const DisconnectCallback& callback) override;

signals:
    void messageReceived(const std::string& message, QUuid senderId, const std::string& username);
    void disconnected(QUuid uuid);

private slots:
    void readUsername();
    void readMessage();
    void onDisconnected();

private:
    std::unique_ptr<QTcpSocket> m_socket;
    QUuid m_uuid;
    std::string m_username;
    MessageCallback m_messageCallback;
    DisconnectCallback m_disconnectCallback;
};