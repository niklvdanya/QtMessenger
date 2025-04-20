#include "auth_handler.h"
#include <QDataStream>

AuthHandler::AuthHandler(QTcpSocket* socket) : m_socket(socket) {}

void AuthHandler::sendUsername(const std::string& username) {
    QDataStream stream(m_socket);
    stream << QString::fromStdString(username);
}