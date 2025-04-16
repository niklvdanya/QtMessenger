#include <QCoreApplication>
#include <QDebug>
#include "multithreaded_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MultithreadedServer server(12345, 4);
    server.start();

    QObject::connect(&server, &MultithreadedServer::newConnection,
                     [](QUuid clientId) {
                         qDebug() << "Новое подключение:" << clientId;
                     });

    QObject::connect(&server, &MultithreadedServer::clientDisconnected,
                     [](QUuid clientId) {
                         qDebug() << "Клиент отключился:" << clientId;
                     });

    QObject::connect(&server, &MultithreadedServer::messageReceived,
                     [](const std::string& message, QUuid senderId, const std::string& username) {
                         qDebug() << "Сообщение от" << QString::fromStdString(username) 
                                  << "(" << senderId << "):" << QString::fromStdString(message);
                     });

    return a.exec();
}