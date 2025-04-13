#include "message.h"

QDataStream& operator<<(QDataStream& stream, const Message& msg) {
    stream << msg.senderId << msg.text << msg.timestamp;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Message& msg) {
    stream >> msg.senderId >> msg.text >> msg.timestamp;
    return stream;
}