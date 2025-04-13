#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QUuid>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    void connectToServer(const QString& host, quint16 port, const QString& username);
    void sendMessage(const QString& message);

    signals:
    void messageReceived(const QString& sender, const QString& message);
private slots:
    void onReadyRead();

private:
    QTcpSocket* m_socket;
    QString m_username;
};