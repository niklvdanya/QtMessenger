#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

class NetworkClient;

class ChatWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ChatWindow(QWidget* parent = nullptr);
    QString username() const; 

private slots:
    void sendMessage();

private:
    QListWidget* m_chatHistory;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;
    NetworkClient* m_networkClient;
    QString m_username;
};