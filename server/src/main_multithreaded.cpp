#include <QCoreApplication>
#include <QDebug>
#include "multithreaded_server.h"
#include "database_manager.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    DatabaseManager dbManager;
    if (!dbManager.initialize()) {
        qDebug() << "Failed to initialize database. Exiting...";
        return -1;
    }
    
    MultithreadedServer server(12345, 4, nullptr);
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