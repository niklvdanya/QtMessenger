#pragma once
#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <QUuid>
#include <string>

class ClientSession : public QObject {
    Q_OBJECT
public:
    explicit ClientSession(std::unique_ptr<QTcpSocket> socket, QObject* parent = nullptr);
    ~ClientSession() override = default;

    QUuid uuid() const noexcept;
    std::string username() const;
    void sendMessage(const std::string& message);

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
};