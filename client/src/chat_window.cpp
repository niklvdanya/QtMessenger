#include "chat_window.h"
#include "network_client_factory.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

ChatWindow::ChatWindow(QWidget* parent) : QMainWindow(parent) {
    m_username = "User" + std::to_string(rand() % 1000);
    setupUi();
    connectSignals();
}

std::string ChatWindow::username() const {
    return m_username;
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
}

void ChatWindow::connectSignals() {
    m_networkClient = NetworkClientFactory::createTcpClient(this);

    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    m_networkClient->setMessageCallback([this](const std::string& sender, const std::string& message) {
        m_chatHistory->addItem(QString::fromStdString(sender + ": " + message));
    });
    m_networkClient->setDisconnectedCallback([this]() {
        m_chatHistory->addItem("Отключено от сервера");
    });

    m_networkClient->connectToServer("127.0.0.1", 12345, m_username);
}

void ChatWindow::sendMessage() {
    std::string message = m_inputField->text().toStdString();
    if (!message.empty()) {
        m_networkClient->sendMessage(message);
        m_inputField->clear();
    }
}