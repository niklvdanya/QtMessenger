#include <QCoreApplication>
#include <QDebug>
#include "multithreaded_server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    MultithreadedServer server(12345, 4);
    server.start(12345);

    QObject::connect(&server, &MultithreadedServer::newConnection,
                     [](QUuid clientId) {
                         qDebug() << "New connection:" << clientId;
                     });

    QObject::connect(&server, &MultithreadedServer::clientDisconnected,
                     [](QUuid clientId) {
                         qDebug() << "Client disconnected:" << clientId;
                     });

    QObject::connect(&server, &MultithreadedServer::messageReceived,
                     [](const std::string& message, QUuid senderId, const std::string& username) {
                         qDebug() << "Message from" << QString::fromStdString(username) 
                                  << "(" << senderId << "):" << QString::fromStdString(message);
                     });

    return a.exec();
}