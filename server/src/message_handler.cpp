#include "message_handler.h"
#include <QDebug>

MessageHandler::MessageHandler(std::shared_ptr<IClientManager> clientManager) 
    : m_clientManager(clientManager) {}

void MessageHandler::handleMessage(const std::string& message, QUuid senderId, const std::string& username) {
    qDebug() << "Handling message from" << QString::fromStdString(username) 
             << "(" << senderId << "):" << QString::fromStdString(message);
    m_clientManager->broadcastMessage(message, senderId, username);
}