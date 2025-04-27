#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool initialize();
    bool addUser(const QString& username, const QString& password);
    bool checkUser(const QString& username, const QString& password);
    bool userExists(const QString& username);

private:
    QSqlDatabase m_db;
};