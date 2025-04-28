#include "login_window.h"
#include "password_window.h"
#include "register_window.h"
#include "chat_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

LoginWindow::LoginWindow(IDatabase* dbManager, QWidget* parent)
    : QMainWindow(parent), m_dbManager(dbManager) {
    setWindowTitle("Qt Chat App - Login");
    setFixedSize(400, 300);
    setupUi();
    applyStyles();
}

void LoginWindow::displayErrorMessage(const QString& message) {
    m_titleLabel->setText(message);
    m_titleLabel->setStyleSheet("color: #dc3545;");
}

void LoginWindow::clearErrorMessage() {
    m_titleLabel->setText("Qt Chat App");
    m_titleLabel->setStyleSheet("color: #333333;");
}

void LoginWindow::setupUi() {
    m_titleLabel = std::make_unique<QLabel>("Qt Chat App", this);
    m_usernameField = std::make_unique<QLineEdit>(this);
    m_loginButton = std::make_unique<QPushButton>("Login", this);
    m_registerButton = std::make_unique<QPushButton>("Register", this);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addWidget(m_titleLabel.get(), 0, Qt::AlignCenter);
    mainLayout->addWidget(m_usernameField.get());
    mainLayout->addWidget(m_loginButton.get());
    mainLayout->addWidget(m_registerButton.get());

    m_usernameField->setPlaceholderText("Enter username...");

    connect(m_loginButton.get(), &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton.get(), &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
}

void LoginWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QLabel {
            font-size: 24px;
            font-weight: bold;
            color: #333333;
            margin-bottom: 20px;
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
    )");
}

void LoginWindow::onLoginClicked() {
    QString username = m_usernameField->text().trimmed();
    if (username.isEmpty()) {
        displayErrorMessage("Please enter a username");
        return;
    }

    if (!m_dbManager->userExists(username)) {
        displayErrorMessage("User does not exist");
        return;
    }

    auto* passwordWindow = new PasswordWindow(username, m_dbManager, this);

    connect(passwordWindow, &PasswordWindow::chatWindowOpened, this, [this](ChatWindow* chatWindow) {
        connect(chatWindow, &ChatWindow::loggedOut, this, [this]() {
            show();
            m_usernameField->clear();
            clearErrorMessage();
        });
        hide();
    });
    
    passwordWindow->exec();
}

void LoginWindow::onRegisterClicked() {
    auto* registerWindow = new RegisterWindow(m_dbManager, this);
    registerWindow->exec();
}