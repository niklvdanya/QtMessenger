#pragma once
#include <memory>

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

#include "database_manager.h"

class IAuthView
{
public:
    virtual ~IAuthView() = default;
    virtual void displayErrorMessage(const QString& message) = 0;
    virtual void clearErrorMessage() = 0;
};

class LoginWindow : public QMainWindow, public IAuthView
{
    Q_OBJECT
public:
    explicit LoginWindow(IDatabase* dbManager, QWidget* parent = nullptr);
    ~LoginWindow() override = default;

    void displayErrorMessage(const QString& message) override;
    void clearErrorMessage() override;

signals:
    void chatWindowClosed();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUi();
    void applyStyles();

    IDatabase* m_dbManager;
    std::unique_ptr<QLineEdit> m_usernameField;
    std::unique_ptr<QPushButton> m_loginButton;
    std::unique_ptr<QPushButton> m_registerButton;
    std::unique_ptr<QLabel> m_titleLabel;
};