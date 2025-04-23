#include "chat_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout> 
#include <QWidget>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDateTime>
#include <QStyle>
#include <QKeyEvent> 

ChatWindow::ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent) 
    : QMainWindow(parent) {
    setWindowTitle("Qt Chat App");
    setMinimumSize(600, 400); 
    setupUi();
    applyStyles();

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
    QString formattedMessage = QString::fromStdString(
        "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] " +
        displayName + ": " + message);

    auto* item = new QListWidgetItem(formattedMessage, m_chatHistory.get());
    if (sender == m_controller->username()) {
        item->setBackground(QColor(200, 230, 255)); 
    }
    m_chatHistory->addItem(item);
    m_chatHistory->scrollToBottom();
}

void ChatWindow::displaySystemMessage(const std::string& message) {
    QString formattedMessage = QString::fromStdString(
        "[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] [System]: " + message);
    auto* item = new QListWidgetItem(formattedMessage, m_chatHistory.get());
    item->setForeground(Qt::darkGray); 
    m_chatHistory->addItem(item);
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
    m_emojiButton = std::make_unique<QPushButton>("😊", this);
    m_statusLabel = std::make_unique<QLabel>("Disconnected", this);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);
    
    auto* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_emojiButton.get());
    inputLayout->addWidget(m_inputField.get());
    inputLayout->addWidget(m_sendButton.get());

    mainLayout->addWidget(m_statusLabel.get());
    mainLayout->addWidget(m_chatHistory.get());
    mainLayout->addLayout(inputLayout);

    m_chatHistory->setWordWrap(true);
    m_inputField->setPlaceholderText("Type your message here...");
    m_statusLabel->setAlignment(Qt::AlignRight);

    mainLayout->setStretchFactor(m_chatHistory.get(), 8);
    mainLayout->setStretchFactor(inputLayout, 1);
}

void ChatWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QListWidget {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 10px;
            font-family: 'Arial', sans-serif;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1px solid #0078d4;
        }
        QPushButton {
            background-color: #0078d4;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #005ea2;
        }
        QPushButton:pressed {
            background-color: #004e8c;
        }
        QPushButton#emojiButton {
            background-color: #ffffff;
            color: black;
            font-size: 20px;
            padding: 8px;
            min-width: 40px;
        }
        QPushButton#emojiButton:hover {
            background-color: #e0e0e0;
        }
        QPushButton#emojiButton:pressed {
            background-color: #d0d0d0;
        }
        QLabel {
            font-size: 12px;
            color: #666666;
        }
    )");

    m_emojiButton->setObjectName("emojiButton"); 
}

void ChatWindow::connectSignals() {
    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_inputField.get(), &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);
    connect(m_emojiButton.get(), &QPushButton::clicked, this, &ChatWindow::openEmojiWindow);  
}

void ChatWindow::sendMessage() {
    QString text = m_inputField->text().trimmed();
    if (!text.isEmpty()) {
        m_controller->sendMessage(text.toStdString());
        clearInput();
    }
}

void ChatWindow::updateConnectionStatus(bool connected) {
    m_statusLabel->setText(connected ? "Connected" : "Disconnected");
    m_statusLabel->setStyleSheet(connected ? "color: #28a745;" : "color: #dc3545;");
}

void ChatWindow::openEmojiWindow() {
    EmojiWindow* emojiWindow = new EmojiWindow(this);
    connect(emojiWindow, &EmojiWindow::emojiSelected, this, &ChatWindow::insertEmoji);
    emojiWindow->exec(); 
}

void ChatWindow::insertEmoji(const QString& emoji) {
    m_inputField->insert(emoji); 
}