#include "chat_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout> 
#include <QWidget>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDateTime>
#include <QStyle>
#include <QKeyEvent>
#include <QMessageBox>
#include "login_window.h"

ChatWindow::ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent) 
    : QMainWindow(parent) {
    setWindowTitle("Qt Chat App");
    setMinimumSize(600, 400); 
    setupUi();
    applyStyles();

    m_controller = std::make_unique<ChatController>(std::move(networkClient), this);
    connectSignals();
}

void ChatWindow::handleMessageReceived(const Message& msg) {
    std::string displayName = (msg.username == m_controller->username()) ? "You" : msg.username;
    QString formattedMessage = QString::fromStdString(
        "[" + msg.timestamp.toString("hh:mm:ss").toStdString() + "] " +
        displayName + ": " + msg.text);
    auto* item = new QListWidgetItem(formattedMessage, m_chatHistory.get());
    
    if (msg.username == m_controller->username()) {
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
    m_logoutButton = std::make_unique<QPushButton>("Log out", this); 
    m_statusLabel = std::make_unique<QLabel>("Disconnected", this);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    auto* topLayout = new QHBoxLayout();
    topLayout->addWidget(m_logoutButton.get());
    topLayout->addStretch(); 
    topLayout->addWidget(m_statusLabel.get());
    
    auto* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_emojiButton.get());
    inputLayout->addWidget(m_inputField.get());
    inputLayout->addWidget(m_sendButton.get());

    mainLayout->addLayout(topLayout);  
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
        QPushButton#logoutButton {
            background-color: #dc3545;  /* Красный цвет для кнопки выхода */
        }
        QPushButton#logoutButton:hover {
            background-color: #c82333;
        }
        QPushButton#logoutButton:pressed {
            background-color: #bd2130;
        }
        QLabel {
            font-size: 12px;
            color: #666666;
        }
    )");

    m_emojiButton->setObjectName("emojiButton");
    m_logoutButton->setObjectName("logoutButton"); 
}

void ChatWindow::connectSignals() {
    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_inputField.get(), &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);
    connect(m_emojiButton.get(), &QPushButton::clicked, this, &ChatWindow::openEmojiWindow);
    connect(m_logoutButton.get(), &QPushButton::clicked, this, &ChatWindow::logout); 
    if (auto* networkClient = m_controller->getNetworkClient()) {
        connect(networkClient, &NetworkClient::messageReceived,
                this, &ChatWindow::handleMessageReceived);
        connect(networkClient, &NetworkClient::connectionStatusChanged,
                this, &ChatWindow::updateConnectionStatus);
    }
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

void ChatWindow::logout() {
    QMessageBox confirmBox;
    confirmBox.setWindowTitle("Confirm Logout");
    confirmBox.setText("Are you sure you want to log out?");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    confirmBox.setIcon(QMessageBox::Question);
    
    int result = confirmBox.exec();
    
    if (result == QMessageBox::Yes) {
        if (auto* networkClient = m_controller->getNetworkClient()) {
            networkClient->disconnect();
        }
        emit loggedOut();
        close();
    }
}