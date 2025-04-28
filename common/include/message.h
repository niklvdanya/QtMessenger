#pragma once
#include <QUuid>
#include <QDateTime>
#include <QDataStream>
#include <string>

enum class MessageType {
    Chat,      
    UserList,  
    System      
};

struct Message {
    QUuid senderId;
    std::string username;
    std::string text;
    QDateTime timestamp;
    MessageType type = MessageType::Chat; 
    std::vector<std::string> userList; 

    bool operator==(const Message& other) const noexcept {
        return senderId == other.senderId &&
               username == other.username &&
               text == other.text &&
               timestamp == other.timestamp;
    }
};

QDataStream& operator<<(QDataStream& stream, const Message& msg);
QDataStream& operator>>(QDataStream& stream, Message& msg);