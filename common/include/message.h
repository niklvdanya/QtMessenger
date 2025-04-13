#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QDataStream> // Для операторов << и >>

struct Message {
    QUuid senderId;
    QString text;
    QDateTime timestamp;
    
    bool operator==(const Message& other) const {
        return senderId == other.senderId && 
               text == other.text && 
               timestamp == other.timestamp;
    }
};

QDataStream& operator<<(QDataStream& stream, const Message& msg);
QDataStream& operator>>(QDataStream& stream, Message& msg);