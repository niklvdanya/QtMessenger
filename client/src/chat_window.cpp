#include "chat_window.h"
#include "network_client.h"
#include "message.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QInputDialog>
#include <QRandomGenerator>

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
    
    bool ok;
    m_username = QInputDialog::getText(this, "Введите имя", 
                                       "Ваше имя:", QLineEdit::Normal, 
                                       "User", &ok);
    if (!ok || m_username.isEmpty()) {
        m_username = "Guest_" + QString::number(QRandomGenerator::global()->bounded(1000));
    }
    
    m_networkClient = new NetworkClient(this);
    m_networkClient->connectToServer("127.0.0.1", 12345, m_username);
    
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_networkClient, &NetworkClient::messageReceived, this, 
        [this](const QString& sender, const QString& message) {
            QString displayName = (sender == m_username) ? "You" : sender;
            m_chatHistory->addItem(QString("[%1] %2: %3")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(displayName)
                .arg(message));
        });
}

QString ChatWindow::username() const {
    return m_username;
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