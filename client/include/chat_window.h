#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include <string>
#include "inetwork_client.h"

class IChatView {
public:
    virtual ~IChatView() = default;
    virtual void displayMessage(const std::string& sender, const std::string& message) = 0;
    virtual void displaySystemMessage(const std::string& message) = 0;
    virtual std::string getInputText() = 0;
    virtual void clearInput() = 0;
};

class ChatWindow : public QMainWindow, public IChatView {
    Q_OBJECT
public:
    explicit ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent = nullptr);
    std::string username() const;
    
    void displayMessage(const std::string& sender, const std::string& message) override;
    void displaySystemMessage(const std::string& message) override;
    std::string getInputText() override;
    void clearInput() override;

private slots:
    void sendMessage();

private:
    void setupUi();
    void connectSignals();

    std::unique_ptr<QListWidget> m_chatHistory;
    std::unique_ptr<QLineEdit> m_inputField;
    std::unique_ptr<QPushButton> m_sendButton;
    std::unique_ptr<INetworkClient> m_networkClient;
    std::string m_username;
};