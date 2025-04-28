#include "chat_window.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

ChatWindow::ChatWindow(std::unique_ptr<INetworkClient> networkClient, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Chat App");
    setMinimumSize(600, 400);
    setupUi();
    applyStyles();

    m_controller = std::make_unique<ChatController>(std::move(networkClient), this);

    connectSignals();
    updateConnectionStatus(false);
}

std::string ChatWindow::getInputText()
{
    return m_inputField->text().trimmed().toStdString();
}

void ChatWindow::clearInput()
{
    m_inputField->clear();
}

void ChatWindow::setupUi()
{
    setWindowTitle("Qt Chat App");
    setMinimumSize(800, 600);

    auto* titleLabel = new QLabel("QtMessenger", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    m_logoutButton = std::make_unique<QPushButton>("Logout", this);
    m_logoutButton->setObjectName("logoutButton");
    m_logoutButton->setCursor(Qt::PointingHandCursor);

    m_statusLabel = std::make_unique<QLabel>("Offline", this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(m_logoutButton.get());
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel.get());
    auto* headerSeparator = new QFrame(this);
    headerSeparator->setFrameShape(QFrame::HLine);
    headerSeparator->setFrameShadow(QFrame::Sunken);
    headerSeparator->setObjectName("headerSeparator");

    m_chatHistory = std::make_unique<QListWidget>(this);
    m_chatHistory->setObjectName("chatHistory");
    m_chatHistory->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_chatHistory->setSelectionMode(QAbstractItemView::NoSelection);
    m_chatHistory->setFocusPolicy(Qt::NoFocus);
    m_chatHistory->setFrameShape(QFrame::NoFrame);

    m_inputField = std::make_unique<QLineEdit>(this);
    m_inputField->setPlaceholderText("Type your message...");
    m_inputField->setObjectName("inputField");
    m_inputField->setMinimumHeight(50);

    QFont emojiFont;
    emojiFont.setFamily("Segoe UI Emoji, Apple Color Emoji, Noto Color Emoji, Segoe UI, Arial");
    emojiFont.setPointSize(14);
    m_inputField->setFont(emojiFont);

    m_emojiButton = std::make_unique<QPushButton>("😊", this);
    m_emojiButton->setObjectName("emojiButton");
    m_emojiButton->setCursor(Qt::PointingHandCursor);
    m_emojiButton->setToolTip("Insert emoji");
    m_emojiButton->setFont(emojiFont);

    m_sendButton = std::make_unique<QPushButton>("→", this);
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setCursor(Qt::PointingHandCursor);
    m_sendButton->setToolTip("Send message");

    m_usersButton = std::make_unique<QPushButton>("👥", this);
    m_usersButton->setObjectName("usersButton");
    m_usersButton->setCursor(Qt::PointingHandCursor);
    m_usersButton->setToolTip("Show users");
    m_usersButton->setFont(emojiFont);

    auto* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_emojiButton.get());
    inputLayout->addWidget(m_inputField.get());
    inputLayout->addWidget(m_sendButton.get());
    inputLayout->addWidget(m_usersButton.get());

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(headerSeparator);
    mainLayout->addWidget(m_chatHistory.get());
    mainLayout->addLayout(inputLayout);

    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_inputField.get(), &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);
    connect(m_emojiButton.get(), &QPushButton::clicked, this, &ChatWindow::openEmojiWindow);
    connect(m_logoutButton.get(), &QPushButton::clicked, this, &ChatWindow::logout);
    connect(m_usersButton.get(), &QPushButton::clicked, this, &ChatWindow::openUserListWindow);

    applyStyles();
}

void ChatWindow::insertEmoji(const QString& emoji)
{
    m_inputField->insert(emoji);
    m_inputField->setFocus();
}

void ChatWindow::displayChatMessage(const Message& msg)
{
    bool isOwnMessage = (msg.username == m_controller->username());
    auto* messageWidget = new QWidget(m_chatHistory.get());
    messageWidget->setObjectName(isOwnMessage ? "ownMessageWidget" : "otherMessageWidget");
    auto* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(5, 5, 5, 5);

    auto* avatarLabel = new QLabel(messageWidget);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setObjectName("avatarLabel");

    QChar firstLetter = QString::fromStdString(msg.username).at(0).toUpper();
    avatarLabel->setText(QString(firstLetter));

    if (isOwnMessage) {
        avatarLabel->setStyleSheet(R"(
            background-color: #179cde;
            color: white;
            border-radius: 20px;
            font-weight: bold;
            font-size: 16px;
        )");
    } else {
        avatarLabel->setStyleSheet(R"(
            background-color: #9e9e9e;
            color: white;
            border-radius: 20px;
            font-weight: bold;
            font-size: 16px;
        )");
    }

    auto* bubbleWidget = new QWidget(messageWidget);
    bubbleWidget->setObjectName(isOwnMessage ? "ownBubble" : "otherBubble");

    if (isOwnMessage) {
        bubbleWidget->setStyleSheet(R"(
            background-color: #e3f2fd;
            border-radius: 18px;
            border-top-right-radius: 5px;
        )");
    } else {
        bubbleWidget->setStyleSheet(R"(
            background-color: white;
            border-radius: 18px;
            border-top-left-radius: 5px;
        )");
    }

    auto* bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(2);

    auto* headerWidget = new QWidget(bubbleWidget);
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 2);
    headerLayout->setSpacing(10);

    auto* nameLabel =
        new QLabel(isOwnMessage ? "You" : QString::fromStdString(msg.username), headerWidget);
    nameLabel->setObjectName("nameLabel");

    if (isOwnMessage) {
        nameLabel->setStyleSheet("font-weight: bold; color: #179cde;");
    } else {
        nameLabel->setStyleSheet("font-weight: bold; color: #333333;");
    }

    auto* timeLabel = new QLabel(msg.timestamp.toString("HH:mm"), headerWidget);
    timeLabel->setObjectName("timeLabel");
    timeLabel->setStyleSheet("color: #9e9e9e; font-size: 11px;");

    headerLayout->addWidget(nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(timeLabel);

    auto* textLabel = new QLabel(QString::fromStdString(msg.text), bubbleWidget);
    textLabel->setObjectName("textLabel");
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QFont messageFont;
    messageFont.setFamily("Segoe UI Emoji, Apple Color Emoji, Noto Color Emoji, Segoe UI, Arial");
    messageFont.setPointSize(14);
    textLabel->setFont(messageFont);
    textLabel->setStyleSheet("color: #333333;");

    bubbleLayout->addWidget(headerWidget);
    bubbleLayout->addWidget(textLabel);

    if (isOwnMessage) {
        messageLayout->addStretch();
        messageLayout->addWidget(bubbleWidget);
        messageLayout->addWidget(avatarLabel);
    } else {
        messageLayout->addWidget(avatarLabel);
        messageLayout->addWidget(bubbleWidget);
        messageLayout->addStretch();
    }

    auto* item = new QListWidgetItem(m_chatHistory.get());
    item->setSizeHint(messageWidget->sizeHint());
    m_chatHistory->addItem(item);
    m_chatHistory->setItemWidget(item, messageWidget);
    m_chatHistory->scrollToBottom();
}

void ChatWindow::displaySystemMessage(const std::string& message)
{
    auto* messageWidget = new QWidget(m_chatHistory.get());
    auto* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(5, 5, 5, 5);

    auto* bubbleWidget = new QWidget(messageWidget);
    bubbleWidget->setObjectName("systemBubble");
    bubbleWidget->setStyleSheet(R"(
        background-color: #f1f1f1;
        border-radius: 18px;
    )");

    auto* bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(15, 10, 15, 10);

    QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
    QString messageText = QString::fromStdString(message);

    auto* textLabel = new QLabel(QString("[%1] %2").arg(timestamp).arg(messageText), bubbleWidget);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("color: #757575; font-size: 13px; font-style: italic;");

    bubbleLayout->addWidget(textLabel);

    messageLayout->addStretch();
    messageLayout->addWidget(bubbleWidget);
    messageLayout->addStretch();

    auto* item = new QListWidgetItem(m_chatHistory.get());
    item->setSizeHint(messageWidget->sizeHint());
    m_chatHistory->addItem(item);
    m_chatHistory->setItemWidget(item, messageWidget);
    m_chatHistory->scrollToBottom();
}

void ChatWindow::applyStyles()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: white;
        }
        
        QLabel#titleLabel {
            font-size: 22px;
            font-weight: bold;
            color: #179cde;
        }
        
        QLabel#statusLabel[text="Connected"] {
            color: #28a745;
            font-weight: bold;
        }
        
        QLabel#statusLabel[text="Offline"] {
            color: #dc3545;
            font-weight: bold;
        }
        
        QFrame#headerSeparator {
            color: #e0e0e0;
            height: 1px;
        }
        
        QListWidget#chatHistory {
            background-color: #f5f7fa;
            border-radius: 10px;
            padding: 10px;
        }
        
        QLineEdit#inputField {
            border: 1px solid #e0e0e0;
            border-radius: 25px;
            padding: 12px 20px;
            font-size: 14px;
            background-color: white;
        }
        
        QLineEdit#inputField:focus {
            border: 1.5px solid #179cde;
        }
        
        QPushButton#sendButton {
            background-color: #179cde;
            color: white;
            border: none;
            border-radius: 25px;
            font-size: 18px;
            min-width: 50px;
            max-width: 50px;
            min-height: 50px;
            max-height: 50px;
        }
        
        QPushButton#sendButton:hover {
            background-color: #0e8fd0;
        }
        
        QPushButton#emojiButton, QPushButton#usersButton {
            background-color: transparent;
            color: #737373;
            border: none;
            border-radius: 25px;
            font-size: 22px;
            min-width: 50px;
            max-width: 50px;
            min-height: 50px;
            max-height: 50px;
        }
        
        QPushButton#emojiButton:hover, QPushButton#usersButton:hover {
            background-color: #f0f2f5;
        }
        
        QPushButton#logoutButton {
            background-color: transparent;
            color: #737373;
            border: 1px solid #d0d0d0;
            border-radius: 20px;
            padding: 5px 15px;
            font-size: 14px;
        }
        
        QPushButton#logoutButton:hover {
            background-color: #f0f2f5;
        }
    )");
}

void ChatWindow::updateConnectionStatus(bool connected)
{
    if (connected) {
        m_statusLabel->setText("Connected");
        m_statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        m_statusLabel->setText("Disconnected");
        m_statusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
}

void ChatWindow::connectSignals()
{
    connect(m_sendButton.get(), &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_inputField.get(), &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);
    connect(m_emojiButton.get(), &QPushButton::clicked, this, &ChatWindow::openEmojiWindow);
    connect(m_logoutButton.get(), &QPushButton::clicked, this, &ChatWindow::logout);
    connect(m_usersButton.get(), &QPushButton::clicked, this, &ChatWindow::openUserListWindow);

    if (auto* networkClient = m_controller->getNetworkClient()) {
        connect(networkClient, &NetworkClient::connectionStatusChanged, this,
                &ChatWindow::updateConnectionStatus);
        connect(networkClient, &NetworkClient::userListReceived, this, &ChatWindow::updateUserList);
    }
}

void ChatWindow::sendMessage()
{
    m_controller->sendMessage();
}

void ChatWindow::openEmojiWindow()
{
    auto* emojiWindow = new EmojiWindow(this);
    connect(emojiWindow, &EmojiWindow::emojiSelected, this, &ChatWindow::insertEmoji);
    emojiWindow->exec();
}

void ChatWindow::logout()
{
    QMessageBox confirmBox;
    confirmBox.setWindowTitle("Confirm Logout");
    confirmBox.setText("Are you sure you want to log out?");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    confirmBox.setIcon(QMessageBox::Question);

    int result = confirmBox.exec();

    if (result == QMessageBox::Yes) {
        m_controller->disconnect();
        emit loggedOut();
        close();
    }
}

void ChatWindow::openUserListWindow()
{
    if (!m_userListWindow) {
        m_userListWindow = std::make_unique<UserListWindow>(this);
    }
    requestUserList();

    m_userListWindow->show();
}

void ChatWindow::requestUserList()
{
    m_controller->requestUserList();
}

void ChatWindow::updateUserList(const std::vector<QString>& userList)
{
    if (m_userListWindow) {
        m_userListWindow->updateUserList(userList);
    }
}