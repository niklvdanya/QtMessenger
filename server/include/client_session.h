#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QUuid>

class ClientSession : public QObject {
    Q_OBJECT
public:
    explicit ClientSession(QTcpSocket* socket, QObject* parent = nullptr);
    ~ClientSession() = default;
    QUuid uuid() const;
    void sendMessage(const QString& message);

signals:
    void messageReceived(const QString& message, QUuid senderId);
    void disconnected(QUuid uuid);

private slots:
    void readMessage();
    void onDisconnected();

private:
    QTcpSocket* m_socket;
    QUuid m_uuid;
};