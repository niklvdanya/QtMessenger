#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <memory>
#include <optional>
#include <stdexcept>

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message);
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool addUser(const QString& username, const QString& password) = 0;
    virtual bool checkUser(const QString& username, const QString& password) = 0;
    virtual bool userExists(const QString& username) = 0;
};

class DatabaseManager : public IDatabase {
public:
    DatabaseManager();
    ~DatabaseManager() override;

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&&) = delete;
    DatabaseManager& operator=(DatabaseManager&&) = delete;

    bool initialize();
    bool addUser(const QString& username, const QString& password) override;
    bool checkUser(const QString& username, const QString& password) override;
    bool userExists(const QString& username) override;

private:
    QSqlDatabase m_db;
};