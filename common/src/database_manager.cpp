#include "database_manager.h"

#include <QDebug>
#include <QDir>
#include <QSqlError>

DatabaseException::DatabaseException(const std::string& message) : std::runtime_error(message) {}

DatabaseManager::DatabaseManager() : m_db(QSqlDatabase::addDatabase("QSQLITE")) {}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize()
{
    qDebug() << "Initializing database in" << QDir::currentPath();

    m_db.setDatabaseName("chat_app.db");
    if (!m_db.open()) {
        const auto errorMessage = m_db.lastError().text();
        qDebug() << "Failed to open database:" << errorMessage;
        throw DatabaseException("Failed to open database: " + errorMessage.toStdString());
    }

    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "username TEXT PRIMARY KEY, "
                    "password TEXT NOT NULL)")) {
        const auto errorMessage = query.lastError().text();
        qDebug() << "Failed to create table:" << errorMessage;
        throw DatabaseException("Failed to create table: " + errorMessage.toStdString());
    }

    query.exec("SELECT COUNT(*) FROM users");
    if (query.next() && query.value(0).toInt() == 0) {
        qDebug() << "No users found, creating test user 'danya' with password '123'";
        addUser("danya", "123");
    }

    return true;
}

bool DatabaseManager::addUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);

    if (!query.exec()) {
        qDebug() << "Failed to add user:" << query.lastError().text();
        return false;
    }

    qDebug() << "User" << username << "added successfully";
    return true;
}

bool DatabaseManager::checkUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        qDebug() << "User" << username << "not found or query error:" << query.lastError().text();
        return false;
    }

    QString storedPassword = query.value(0).toString();
    bool match = (storedPassword == password);

    qDebug() << "Password check for" << username << "- match:" << match;

    return match;
}

bool DatabaseManager::userExists(const QString& username)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        qDebug() << "Error checking if user exists:" << query.lastError().text();
        return false;
    }

    bool exists = query.value(0).toInt() > 0;
    qDebug() << "User" << username << (exists ? "exists" : "does not exist");
    return exists;
}