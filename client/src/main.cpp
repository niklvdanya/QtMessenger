#include "login_window.h"
#include "database_manager.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    DatabaseManager dbManager;
    if (!dbManager.initialize()) {
        qDebug() << "Failed to initialize database. Exiting...";
        return -1;
    }

    LoginWindow window(&dbManager);
    window.show();

    return app.exec();
}