// main.cpp
#include "chat_window.h"
#include "network_client_factory.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    auto networkClient = NetworkClientFactory::createTcpClient(nullptr);

    ChatWindow window(std::move(networkClient));
    window.show();
    
    return app.exec();
}