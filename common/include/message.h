#pragma once
#include <optional>
#include <string>
#include <vector>

#include <QDataStream>
#include <QDateTime>
#include <QUuid>

enum class MessageType
{
    Chat,
    UserList,
    System
};

class Message
{
public:
    Message() = default;
    explicit Message(const QUuid& senderId, std::string username, std::string text);

    QUuid senderId;
    std::string username;
    std::string text;
    QDateTime timestamp;
    MessageType type = MessageType::Chat;
    std::vector<std::string> userList;

    bool operator==(const Message& other) const noexcept;
    bool operator!=(const Message& other) const noexcept;
};

QDataStream& operator<<(QDataStream& stream, const Message& msg);
QDataStream& operator>>(QDataStream& stream, Message& msg);