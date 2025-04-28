#include "message.h"

Message::Message(const QUuid& senderId, std::string username, std::string text)
    : senderId(senderId)
    , username(std::move(username))
    , text(std::move(text))
    , timestamp(QDateTime::currentDateTime())
    , type(MessageType::Chat) {
}

bool Message::operator==(const Message& other) const noexcept {
    return senderId == other.senderId &&
           username == other.username &&
           text == other.text &&
           timestamp == other.timestamp &&
           type == other.type;
}

bool Message::operator!=(const Message& other) const noexcept {
    return !(*this == other);
}

QDataStream& operator<<(QDataStream& stream, const Message& msg) {
    stream.setVersion(QDataStream::Qt_6_0);
    QString username = QString::fromStdString(msg.username);
    QString text = QString::fromStdString(msg.text);
    stream << msg.senderId << username << text << msg.timestamp
           << static_cast<int>(msg.type);
    
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
        msg.userList.reserve(size);
        
        for (int i = 0; i < size; ++i) {
            QString user;
            stream >> user;
            msg.userList.push_back(user.toStdString());
        }
    }
    return stream;
}