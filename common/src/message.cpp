#include "message.h"

QDataStream& operator<<(QDataStream& stream, const Message& msg) {
    stream.setVersion(QDataStream::Qt_6_0);
    stream << msg.senderId << QString::fromStdString(msg.username) 
           << QString::fromStdString(msg.text) << msg.timestamp;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Message& msg) {
    stream.setVersion(QDataStream::Qt_6_0); 
    QString username, text;
    stream >> msg.senderId >> username >> text >> msg.timestamp;
    msg.username = username.toStdString();
    msg.text = text.toStdString();
    return stream;
}