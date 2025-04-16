#include "multithreaded_server.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    MultithreadedServer server(12345, 4);
    
    QObject::connect(&server, &MultithreadedServer::newConnection,
        [](QUuid clientId) {
            qDebug() << "Новое подключение:" << clientId;
        });
    
    QObject::connect(&server, &MultithreadedServer::clientDisconnected,
        [](QUuid clientId) {
            qDebug() << "Отключение клиента:" << clientId;
        });
    
    QObject::connect(&server, &MultithreadedServer::messageReceived,
        [](const QString& message, QUuid senderId, const QString& username) {
            qDebug() << "Сообщение от" << username << "(" << senderId << "):" << message;
        });
    
    server.start();
    
    return app.exec();
} 