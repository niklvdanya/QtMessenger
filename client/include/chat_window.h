#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include <string>
#include <vector>
#include "chat_controller.h"
#include "emoji_window.h"
#include "message.h"
#include "user_list_window.h"

class ChatWindow : public QMainWindow, public IChatView {
    Q_OBJECT
public:
    explicit ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent = nullptr);
    ~ChatWindow() override = default;
    void displaySystemMessage(const std::string& message) override;
    void displayChatMessage(const Message& message) override;
    std::string getInputText() override;
    void clearInput() override;
    void updateConnectionStatus(bool connected) override;
    void updateUserList(const std::vector<QString>& userList) override;
    ChatController* getController() const { return m_controller.get(); }

public slots:
    void sendMessage();
    void openEmojiWindow();
    void insertEmoji(const QString& emoji);
    void logout();
    void openUserListWindow();
    void requestUserList();

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
    std::unique_ptr<QPushButton> m_usersButton;
    std::unique_ptr<QLabel> m_statusLabel;
    std::unique_ptr<ChatController> m_controller;
    std::unique_ptr<UserListWindow> m_userListWindow;
};