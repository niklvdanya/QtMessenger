#pragma once
#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <string_view>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() override = default;

    void connectToServer(std::string_view host, uint16_t port, std::string_view username);
    void sendMessage(std::string_view message);

signals:
    void messageReceived(const std::string& sender, const std::string& message);
    void disconnected();

private slots:
    void onConnected();
    void onReadyRead();

private:
    std::unique_ptr<QTcpSocket> m_socket;
    std::string m_username;
};