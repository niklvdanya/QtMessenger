#pragma once
#include <string>
#include <QTcpSocket>

class AuthHandler {
public:
    explicit AuthHandler(QTcpSocket* socket);
    void sendUsername(const std::string& username);
    void sendPassword(const std::string& password);
    void sendCredentials(const std::string& username, const std::string& password);

private:
    QTcpSocket* m_socket;
};