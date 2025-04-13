#include "chat_window.h"
#include "network_client.h"
#include "message.h"
#include <QVBoxLayout>
#include <QDebug>

ChatWindow::ChatWindow(QWidget* parent) : QMainWindow(parent) {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    m_chatHistory = new QListWidget(this);
    m_inputField = new QLineEdit(this);
    m_sendButton = new QPushButton("Send", this);
    
    layout->addWidget(m_chatHistory);
    layout->addWidget(m_inputField);
    layout->addWidget(m_sendButton);
    
    setCentralWidget(centralWidget);
    
    m_networkClient = new NetworkClient(this);
    m_networkClient->connectToServer("127.0.0.1", 12345);
    
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_networkClient, &NetworkClient::messageReceived, this, 
        [this](const QString& message) {
            m_chatHistory->addItem(QString("[%1] Server: %2")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(message));
        });
}

void ChatWindow::sendMessage() {
    QString message = m_inputField->text();
    if (!message.isEmpty()) {
        m_chatHistory->addItem(QString("[%1] You: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg(message));
        m_networkClient->sendMessage(message);
        m_inputField->clear();
    }
}