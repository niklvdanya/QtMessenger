#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include <string>
#include "chat_controller.h"
#include "emoji_window.h"
#include "message.h"

class IChatView {
public:
    virtual ~IChatView() = default;
    virtual void displaySystemMessage(const std::string& message) = 0;
    virtual std::string getInputText() = 0;
    virtual void clearInput() = 0;
};

class ChatWindow : public QMainWindow, public IChatView {
    Q_OBJECT
public:
    explicit ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent = nullptr);
    void displaySystemMessage(const std::string& message) override;
    std::string getInputText() override;
    void clearInput() override;

public slots:
    void sendMessage();
    void updateConnectionStatus(bool connected);
    void openEmojiWindow();
    void insertEmoji(const QString& emoji);
    void handleMessageReceived(const Message& msg);
    void logout();  
 
signals:
    void loggedOut();  

private:
    void setupUi();
    void connectSignals();
    void applyStyles();

    std::unique_ptr<QListWidget> m_chatHistory;
    std::unique_ptr<QLineEdit> m_inputField;
    std::unique_ptr<QPushButton> m_sendButton;
    std::unique_ptr<QPushButton> m_emojiButton;
    std::unique_ptr<QPushButton> m_logoutButton;  
    std::unique_ptr<QLabel> m_statusLabel;
    std::unique_ptr<ChatController> m_controller;
};