#pragma once
#include <memory>
#include <string>

#include <QTcpSocket>

class IAuthHandler
{
public:
    virtual ~IAuthHandler() = default;
    virtual void sendUsername(const std::string& username) = 0;
    virtual void sendPassword(const std::string& password) = 0;
    virtual void sendCredentials(const std::string& username, const std::string& password) = 0;
};

class AuthHandler : public IAuthHandler
{
public:
    explicit AuthHandler(QTcpSocket* socket);

    void sendUsername(const std::string& username) override;
    void sendPassword(const std::string& password) override;
    void sendCredentials(const std::string& username, const std::string& password) override;

private:
    QTcpSocket* m_socket;
};