#include "chat_window.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDateTime>

ChatWindow::ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent) 
    : QMainWindow(parent)
    , m_networkClient(std::move(networkClient)) {
    setupUi();

    bool ok;
    m_username = QInputDialog::getText(this, "Введите имя", 
                                       "Ваше имя:", QLineEdit::Normal, 
                                       "User", &ok).toStdString();
    if (!ok || m_username.empty()) {
        m_username = "Guest_" + std::to_string(QRandomGenerator::global()->bounded(1000));
    }

    connectSignals();
}

std::string ChatWindow::username() const {
    return m_username;
}

void ChatWindow::displayMessage(const std::string& sender, const std::string& message) {
    std::string displayName = (sender == m_username) ? "You" : sender;
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
    
    m_networkClient->setMessageCallback([this](const std::string& sender, const std::string& message) {
        displayMessage(sender, message);
    });
    
    m_networkClient->setDisconnectedCallback([this]() {
        displaySystemMessage("Отключено от сервера");
    });

    m_networkClient->connectToServer("127.0.0.1", 12345, m_username);
}

void ChatWindow::sendMessage() {
    std::string message = getInputText();
    if (!message.empty()) {
        m_chatHistory->addItem(QString::fromStdString(
            "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] You: " + message));
        m_chatHistory->scrollToBottom();
        m_networkClient->sendMessage(message);
        clearInput();
    }
}