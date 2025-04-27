#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "database_manager.h"

class LoginWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LoginWindow(DatabaseManager* dbManager, QWidget* parent = nullptr);

signals:
    void chatWindowClosed();  

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUi();
    void applyStyles();

    DatabaseManager* m_dbManager;
    std::unique_ptr<QLineEdit> m_usernameField;
    std::unique_ptr<QPushButton> m_loginButton;
    std::unique_ptr<QPushButton> m_registerButton;
    std::unique_ptr<QLabel> m_titleLabel;
};