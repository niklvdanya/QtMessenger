#include "chat_window.h"
#include "network_client.h"
#include "message.h"
#include <QVBoxLayout>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDebug>

ChatWindow::ChatWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    connectSignals();

    bool ok;
    m_username = QInputDialog::getText(this, "Введите имя", 
                                       "Ваше имя:", QLineEdit::Normal, 
                                       "User", &ok).toStdString();
    if (!ok || m_username.empty()) {
        m_username = "Guest_" + std::to_string(QRandomGenerator::global()->bounded(1000));
    }

    m_networkClient->connectToServer("127.0.0.1", 12345, m_username);
}

std::string ChatWindow::username() const {
    return m_username;
}

void ChatWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_chatHistory = std::make_unique<QListWidget>(this);
    m_inputField = std::make_unique<QLineEdit>(this);
    m_sendButton = std::make_unique<QPushButton>("Send", this);

    layout->addWidget(m_chatHistory.get());
    layout->addWidget(m_inputField.get());
    layout->addWidget(m_sendButton.get());

    setCentralWidget(centralWidget);
}

void ChatWindow::connectSignals() {
    m_networkClient = std::make_unique<NetworkClient>(this);

    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_networkClient.get(), &NetworkClient::messageReceived, this, 
        [this](const std::string& sender, const std::string& message) {
            std::string displayName = (sender == m_username) ? "You" : sender;
            m_chatHistory->addItem(QString::fromStdString(
                "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] " +
                displayName + ": " + message));
        });
}

void ChatWindow::sendMessage() {
    std::string message = m_inputField->text().toStdString();
    if (!message.empty()) {
        m_chatHistory->addItem(QString::fromStdString(
            "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] You: " + 
            message));
        m_networkClient->sendMessage(message);
        m_inputField->clear();
    }
}