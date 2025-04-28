#include "message_handler.h"

#include <QDateTime>
#include <QDebug>
#include <QUuid>

MessageHandler::MessageHandler(std::shared_ptr<IClientManager> clientManager)
    : m_clientManager(std::move(clientManager))
{}

void MessageHandler::handleMessage(const Message& message, QUuid senderId)
{
    if (message.type == MessageType::System && message.text == "REQUEST_USER_LIST") {
        handleUserListRequest(senderId);
    } else {
        handleChatMessage(message);
    }
}

void MessageHandler::handleUserListRequest(QUuid senderId)
{
    auto usernames = m_clientManager->getUsernames();

    Message response;
    response.senderId = QUuid::createUuid();
    response.username = "Server";
    response.text = "User list";
    response.timestamp = QDateTime::currentDateTime();
    response.type = MessageType::UserList;
    response.userList = usernames;

    qDebug() << "Sending user list to client" << senderId << "with" << usernames.size() << "users";

    m_clientManager->sendMessageToClient(senderId, response);
}

void MessageHandler::handleChatMessage(const Message& message)
{
    qDebug() << "Broadcasting message from" << QString::fromStdString(message.username) << "("
             << message.senderId << "):" << QString::fromStdString(message.text);

    m_clientManager->broadcastMessage(message);
}