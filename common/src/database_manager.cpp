#include "database_manager.h"
#include <QSqlError>
#include <QDebug>
#include <QDir>

DatabaseManager::DatabaseManager() : m_db(QSqlDatabase::addDatabase("QSQLITE")) {}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize() {
    m_db.setDatabaseName("chat_app.db");
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "username TEXT PRIMARY KEY, "
               "password TEXT NOT NULL)");
    if (query.lastError().isValid()) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::addUser(const QString& username, const QString& password) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if (!query.exec()) {
        qDebug() << "Failed to add user:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::checkUser(const QString& username, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toString() == password;
}

bool DatabaseManager::userExists(const QString& username) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toInt() > 0;
}