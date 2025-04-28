#include <memory>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QStyle>
#include <QStyleFactory>

#include "database_manager.h"
#include "login_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Fusion"));
    QFont defaultFont;
    defaultFont.setFamily("Segoe UI");
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

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