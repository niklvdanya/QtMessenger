#include "message_handler.h"
#include <QDebug>

MessageHandler::MessageHandler(std::shared_ptr<IClientManager> clientManager) 
    : m_clientManager(clientManager) {}

void MessageHandler::handleMessage(const std::string& message, QUuid senderId, const std::string& username) {
    if (message == "REQUEST_USER_LIST") {
        qDebug() << "User list request from" << QString::fromStdString(username) << "(" << senderId << ")";
        
        auto usernames = m_clientManager->getUsernames();
        Message response;
        response.senderId = QUuid::createUuid();
        response.username = "Server";
        response.text = "User list";
        response.timestamp = QDateTime::currentDateTime();
        response.type = MessageType::UserList;
        response.userList = usernames;
        
        m_clientManager->sendMessageToClient(senderId, response);
        return;
    }
    
    qDebug() << "Handling message from" << QString::fromStdString(username) 
             << "(" << senderId << "):" << QString::fromStdString(message);
    m_clientManager->broadcastMessage(message, senderId, username);
}
