#include <memory>

#include <QCoreApplication>
#include <QDebug>

#include "database_manager.h"
#include "multithreaded_server.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    try {
        auto dbManager = std::make_unique<DatabaseManager>();
        if (!dbManager->initialize()) {
            qDebug() << "Failed to initialize database. Exiting...";
            return -1;
        }

        const unsigned short port = 12345;
        const int thread_count = 4;

        MultithreadedServer server(port, thread_count, dbManager.get());

        QObject::connect(&server, &MultithreadedServer::newConnection,
                         [](QUuid clientId) { qDebug() << "New connection:" << clientId; });

        QObject::connect(&server, &MultithreadedServer::clientDisconnected,
                         [](QUuid clientId) { qDebug() << "Client disconnected:" << clientId; });

        QObject::connect(
            &server, &MultithreadedServer::messageReceived, [](const Message& message) {
                qDebug() << "Message from" << QString::fromStdString(message.username) << "("
                         << message.senderId << "):" << QString::fromStdString(message.text);
            });

        server.start(port);

        qDebug() << "Server started on port" << port;

        return a.exec();
    } catch (const DatabaseException& e) {
        qDebug() << "Database error:" << e.what();
        return -1;
    } catch (const std::exception& e) {
        qDebug() << "Error:" << e.what();
        return -1;
    }
}