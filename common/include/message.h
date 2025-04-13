#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QDataStream>

struct Message {
    QUuid senderId;
    QString username;
    QString text;
    QDateTime timestamp;
    
    bool operator==(const Message& other) const {
        return senderId == other.senderId && 
               username == other.username &&
               text == other.text && 
               timestamp == other.timestamp;
    }
};

QDataStream& operator<<(QDataStream& stream, const Message& msg);
QDataStream& operator>>(QDataStream& stream, Message& msg);