#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include <string>
#include "network_client.h"

class ChatWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ChatWindow(QWidget* parent = nullptr);
    std::string username() const;

private slots:
    void sendMessage();

private:
    void setupUi();
    void connectSignals();

    std::unique_ptr<QListWidget> m_chatHistory;
    std::unique_ptr<QLineEdit> m_inputField;
    std::unique_ptr<QPushButton> m_sendButton;
    std::unique_ptr<NetworkClient> m_networkClient;
    std::string m_username;
};