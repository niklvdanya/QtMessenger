#pragma once
#include <memory>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "chat_window.h"
#include "database_manager.h"
#include "inetwork_client.h"

class PasswordWindow : public QDialog
{
    Q_OBJECT
public:
    PasswordWindow(const QString& username, IDatabase* dbManager, QWidget* parent = nullptr);
    ~PasswordWindow() override = default;

signals:
    void chatWindowOpened(ChatWindow* chatWindow);

private slots:
    void onSubmitClicked();

private:
    void setupUi();
    void applyStyles();
    void showError(const QString& message);

    QString m_username;
    IDatabase* m_dbManager;
    std::unique_ptr<QLineEdit> m_passwordField;
    std::unique_ptr<QPushButton> m_submitButton;
    std::unique_ptr<QLabel> m_statusLabel;
};