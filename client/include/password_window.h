#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "database_manager.h"
#include "chat_window.h"
#include "inetwork_client.h"

class PasswordWindow : public QDialog {
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