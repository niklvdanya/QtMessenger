#include "auth_handler.h"
#include <QDataStream>
#include <QDebug>

AuthHandler::AuthHandler(QTcpSocket* socket) : m_socket(socket) {}

void AuthHandler::sendUsername(const std::string& username) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << QString::fromStdString(username);
    
    m_socket->write(block);
    m_socket->flush();
    qDebug() << "Sent username:" << QString::fromStdString(username);
}

void AuthHandler::sendPassword(const std::string& password) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << QString::fromStdString(password);
    
    m_socket->write(block);
    m_socket->flush();
    qDebug() << "Sent password with length:" << password.length();
}

void AuthHandler::sendCredentials(const std::string& username, const std::string& password) {
    // Send both username and password in a single message with clear structure
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    
    // We'll send:
    // 1. The username
    // 2. The password
    out << QString::fromStdString(username) << QString::fromStdString(password);
    
    qDebug() << "Sending credentials - username:" << QString::fromStdString(username) 
             << ", password length:" << password.length();
    
    // Send the entire block at once
    m_socket->write(block);
    m_socket->flush();
}