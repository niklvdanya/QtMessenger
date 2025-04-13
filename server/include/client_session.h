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
    QString username() const; 
    void sendMessage(const QString& message);

signals:
    void messageReceived(const QString& message, QUuid senderId, const QString& username); 
    void disconnected(QUuid uuid);

private slots:
    void readMessage();
    void readUsername();
    void onDisconnected();

private:
    QTcpSocket* m_socket;
    QUuid m_uuid;
    QString m_username; 
};