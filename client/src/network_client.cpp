#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent), m_socket(new QTcpSocket(this)) {
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, []() {
        qDebug() << "Клиент подключился к серверу!";
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError) {
        qDebug() << "Ошибка подключения!";
    });
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    qDebug() << "Попытка подключения к" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void NetworkClient::sendMessage(const QString& message) {
    Message msg;
    msg.senderId = QUuid::createUuid();
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();
    QDataStream stream(m_socket);
    stream << msg;
    qDebug() << "Отправлено сообщение:" << message;
}

void NetworkClient::onReadyRead() {
    QDataStream stream(m_socket);
    Message msg;
    stream >> msg;
    qDebug() << "Получено сообщение:" << msg.text << "в" << msg.timestamp;
    emit messageReceived(msg.text);
}