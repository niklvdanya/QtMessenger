#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "database_manager.h"

class RegisterWindow : public QDialog {
    Q_OBJECT
public:
    explicit RegisterWindow(IDatabase* dbManager, QWidget* parent = nullptr);
    ~RegisterWindow() override = default;

private slots:
    void onRegisterClicked();

private:
    void setupUi();
    void applyStyles();
    void showMessage(const QString& message, bool isError);

    IDatabase* m_dbManager;
    std::unique_ptr<QLineEdit> m_usernameField;
    std::unique_ptr<QLineEdit> m_passwordField;
    std::unique_ptr<QPushButton> m_registerButton;
    std::unique_ptr<QLabel> m_statusLabel;
};