#include "message.h"

QDataStream& operator<<(QDataStream& stream, const Message& msg) {
    stream.setVersion(QDataStream::Qt_6_0);
    stream << msg.senderId << QString::fromStdString(msg.username) 
           << QString::fromStdString(msg.text) << msg.timestamp
           << static_cast<int>(msg.type);
    
    // Если это список пользователей, сериализуем и его
    if (msg.type == MessageType::UserList) {
        stream << static_cast<int>(msg.userList.size());
        for (const auto& user : msg.userList) {
            stream << QString::fromStdString(user);
        }
    }
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Message& msg) {
    stream.setVersion(QDataStream::Qt_6_0); 
    QString username, text;
    int messageType;
    stream >> msg.senderId >> username >> text >> msg.timestamp >> messageType;
    msg.username = username.toStdString();
    msg.text = text.toStdString();
    msg.type = static_cast<MessageType>(messageType);
    
    if (msg.type == MessageType::UserList) {
        int size;
        stream >> size;
        msg.userList.clear();
        for (int i = 0; i < size; ++i) {
            QString user;
            stream >> user;
            msg.userList.push_back(user.toStdString());
        }
    }
    return stream;
}