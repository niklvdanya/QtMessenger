#pragma once
#include <string>
#include <QTcpSocket>

class AuthHandler {
public:
    explicit AuthHandler(QTcpSocket* socket);
    void sendUsername(const std::string& username);

private:
    QTcpSocket* m_socket;
};