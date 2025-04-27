#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "database_manager.h"
#include "chat_window.h"
#include "inetwork_client.h"

class PasswordWindow : public QDialog {
    Q_OBJECT
public:
    PasswordWindow(const QString& username, DatabaseManager* dbManager, QWidget* parent = nullptr);

private slots:
    void onSubmitClicked();

private:
    void setupUi();
    void applyStyles();

    QString m_username;
    DatabaseManager* m_dbManager;
    std::unique_ptr<QLineEdit> m_passwordField;
    std::unique_ptr<QPushButton> m_submitButton;
    std::unique_ptr<QLabel> m_statusLabel;
};