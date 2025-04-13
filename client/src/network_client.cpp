#include "network_client.h"
#include "message.h"
#include <QDataStream>
#include <QDebug>

NetworkClient::NetworkClient(QObject* parent) 
    : QObject(parent), m_socket(new QTcpSocket(this)), m_username("Guest") {
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, []() {
        qDebug() << "Клиент подключился к серверу!";
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError) {
        qDebug() << "Ошибка подключения!";
    });
}

void NetworkClient::connectToServer(const QString& host, quint16 port, const QString& username) {
    m_username = username;
    qDebug() << "Попытка подключения к" << host << ":" << port << "с именем" << m_username;
    m_socket->connectToHost(host, port);
    
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        QDataStream stream(m_socket);
        stream << m_username;
        qDebug() << "Отправлено имя:" << m_username;
    }, Qt::SingleShotConnection);
}

void NetworkClient::sendMessage(const QString& message) {
    Message msg;
    msg.senderId = QUuid::createUuid();
    msg.username = m_username;
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
    qDebug() << "Получено сообщение:" << msg.text << "от" << msg.username;
    emit messageReceived(msg.username, msg.text); 
}