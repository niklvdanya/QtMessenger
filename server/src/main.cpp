#include <QCoreApplication>

#include "async_tcp_server.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    AsyncTcpServer server;
    server.start(12345);
    return app.exec();
}