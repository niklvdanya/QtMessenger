#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "database_manager.h"

class RegisterWindow : public QDialog {
    Q_OBJECT
public:
    explicit RegisterWindow(DatabaseManager* dbManager, QWidget* parent = nullptr);

private slots:
    void onRegisterClicked();

private:
    void setupUi();
    void applyStyles();

    DatabaseManager* m_dbManager;
    std::unique_ptr<QLineEdit> m_usernameField;
    std::unique_ptr<QLineEdit> m_passwordField;
    std::unique_ptr<QPushButton> m_registerButton;
    std::unique_ptr<QLabel> m_statusLabel;
};