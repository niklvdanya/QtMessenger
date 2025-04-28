#include "login_window.h"
#include "database_manager.h"
#include <QApplication>
#include <QDebug>
#include <memory>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    try {
        auto dbManager = std::make_unique<DatabaseManager>();
        if (!dbManager->initialize()) {
            qDebug() << "Failed to initialize database. Exiting...";
            return -1;
        }

        LoginWindow window(dbManager.get());
        window.show();

        return app.exec();
    } catch (const DatabaseException& e) {
        qDebug() << "Database error:" << e.what();
        return -1;
    } catch (const std::exception& e) {
        qDebug() << "Error:" << e.what();
        return -1;
    }
}