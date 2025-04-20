#include "chat_window.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDateTime>

ChatWindow::ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent) 
    : QMainWindow(parent) {
    setupUi();

    bool ok;
    QString username = QInputDialog::getText(this, "Enter name", 
                                             "Your name:", QLineEdit::Normal, 
                                             "User", &ok);
    if (!ok || username.isEmpty()) {
        username = "Guest_" + QString::number(QRandomGenerator::global()->bounded(1000));
    }

    m_controller = std::make_unique<ChatController>(std::move(networkClient), this);
    m_controller->setUsername(username.toStdString());
    
    connectSignals();
}

void ChatWindow::displayMessage(const std::string& sender, const std::string& message) {
    std::string displayName = (sender == m_controller->username()) ? "You" : sender;
    m_chatHistory->addItem(QString::fromStdString(
        "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] " +
        displayName + ": " + message));
    m_chatHistory->scrollToBottom();
}

void ChatWindow::displaySystemMessage(const std::string& message) {
    m_chatHistory->addItem(QString::fromStdString(
        "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] [System]: " + message));
    m_chatHistory->scrollToBottom();
}

std::string ChatWindow::getInputText() {
    return m_inputField->text().toStdString();
}

void ChatWindow::clearInput() {
    m_inputField->clear();
}

void ChatWindow::setupUi() {
    m_chatHistory = std::make_unique<QListWidget>(this);
    m_inputField = std::make_unique<QLineEdit>(this);
    m_sendButton = std::make_unique<QPushButton>("Send", this);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* layout = new QVBoxLayout(centralWidget);
    layout->addWidget(m_chatHistory.get());
    layout->addWidget(m_inputField.get());
    layout->addWidget(m_sendButton.get());

    m_chatHistory->setWordWrap(true);
    m_inputField->setPlaceholderText("Type your message here...");
}

void ChatWindow::connectSignals() {
    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
}

void ChatWindow::sendMessage() {
    m_controller->sendMessage(getInputText());
    clearInput();
}