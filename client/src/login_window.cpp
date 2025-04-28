#include "login_window.h"

#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "chat_window.h"
#include "password_window.h"
#include "register_window.h"

LoginWindow::LoginWindow(IDatabase* dbManager, QWidget* parent)
    : QMainWindow(parent), m_dbManager(dbManager)
{
    setWindowTitle("Qt Chat App - Login");
    setFixedSize(400, 300);
    setupUi();
    applyStyles();
}
void LoginWindow::setupUi()
{
    // Window size
    setFixedSize(450, 650);

    // Create widgets with proper object names
    m_titleLabel = std::make_unique<QLabel>("QtMessenger", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleLabel");

    auto* infoLabel = new QLabel("Simple and convenient messenger", this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setObjectName("infoLabel");

    auto* usernameContainer = new QWidget(this);
    auto* usernameLayout = new QHBoxLayout(usernameContainer);
    usernameLayout->setContentsMargins(0, 0, 0, 0);
    usernameLayout->setSpacing(10);

    auto* usernameIcon = new QLabel("👤", this);
    usernameIcon->setFixedSize(30, 30);
    usernameIcon->setAlignment(Qt::AlignCenter);
    usernameIcon->setObjectName("usernameIcon");

    m_usernameField = std::make_unique<QLineEdit>(this);
    m_usernameField->setPlaceholderText("Username");
    m_usernameField->setMinimumHeight(50);

    usernameLayout->addWidget(usernameIcon);
    usernameLayout->addWidget(m_usernameField.get());

    m_loginButton = std::make_unique<QPushButton>("Login", this);
    m_loginButton->setMinimumHeight(50);
    m_loginButton->setCursor(Qt::PointingHandCursor);

    m_registerButton = std::make_unique<QPushButton>("Register", this);
    m_registerButton->setMinimumHeight(50);
    m_registerButton->setCursor(Qt::PointingHandCursor);

    auto* versionLabel = new QLabel("Version 1.0.0", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setObjectName("versionLabel");

    auto* copyrightLabel = new QLabel("© 2025 Daniil Nikolaev", this);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setObjectName("copyrightLabel");

    // Create layouts
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);

    auto* formContainer = new QWidget(this);
    formContainer->setObjectName("formContainer");
    formContainer->setFixedWidth(380); // Slightly wider for error messages

    auto* formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(30, 30, 30, 30);

    // Add widgets to layouts
    formLayout->addWidget(m_titleLabel.get(), 0, Qt::AlignCenter);
    formLayout->addWidget(infoLabel, 0, Qt::AlignCenter);
    formLayout->addSpacing(20);
    formLayout->addWidget(usernameContainer);
    formLayout->addSpacing(15);
    formLayout->addWidget(m_loginButton.get());
    formLayout->addWidget(m_registerButton.get());

    auto* bottomContainer = new QWidget(this);
    auto* bottomLayout = new QVBoxLayout(bottomContainer);
    bottomLayout->addWidget(versionLabel);
    bottomLayout->addWidget(copyrightLabel);
    bottomLayout->setSpacing(5);

    mainLayout->addStretch();
    mainLayout->addWidget(formContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(bottomContainer);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Connect signals
    connect(m_loginButton.get(), &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton.get(), &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);

    // Apply styles
    applyStyles();
}

void LoginWindow::applyStyles()
{
    // Main window style
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f0f2f5;
        }
        
        QLabel#infoLabel {
            color: #737373;
            font-size: 14px;
            margin-bottom: 10px;
        }
        
        QLabel#titleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #179cde;
            margin-bottom: 10px;
        }
        
        /* Style for error state of titleLabel */
        QLabel#titleLabel[errorState="true"] {
            color: #dc3545;
            font-size: 18px;
        }
        
        QLabel#versionLabel, QLabel#copyrightLabel {
            font-size: 12px;
            color: #9E9E9E;
            margin: 3px 0;
        }
        
        QLabel#usernameIcon {
            font-size: 18px; 
            color: #179cde;
        }
        
        QWidget#formContainer {
            background-color: white;
            border-radius: 30px;
            padding: 20px;
        }
        
        QLineEdit {
            border: 1px solid #e0e0e0;
            border-radius: 25px;
            padding: 12px 20px;
            font-size: 14px;
            background-color: #f5f7fa;
        }
        
        QLineEdit:focus {
            border: 2px solid #179cde;
        }
        
        QPushButton {
            border-radius: 25px;
            font-size: 16px;
            margin: 8px 0;
        }
        
        QPushButton[text="Login"] {
            background-color: #179cde;
            color: white;
            border: none;
            font-weight: bold;
        }
        
        QPushButton[text="Login"]:hover {
            background-color: #0e8fd0;
        }
        
        QPushButton[text="Login"]:pressed {
            background-color: #0980c0;
        }
        
        QPushButton[text="Register"] {
            background-color: transparent;
            color: #179cde;
            border: 1px solid #179cde;
        }
        
        QPushButton[text="Register"]:hover {
            background-color: rgba(23, 156, 222, 0.1);
        }
        
        QPushButton[text="Register"]:pressed {
            background-color: rgba(23, 156, 222, 0.2);
        }
    )");
}

void LoginWindow::displayErrorMessage(const QString& message)
{
    m_titleLabel->setText(message);
    m_titleLabel->setProperty("errorState", true);

    // Force style update for the error state
    m_titleLabel->setStyleSheet("color: #dc3545;"
                                "font-size: 18px;");
}

void LoginWindow::clearErrorMessage()
{
    m_titleLabel->setText("QtMessenger");
    m_titleLabel->setProperty("errorState", false);

    // Reset to normal style
    m_titleLabel->setStyleSheet("font-size: 28px;"
                                "font-weight: bold;"
                                "color: #179cde;"
                                "margin-bottom: 10px;");
}
void LoginWindow::onLoginClicked()
{
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

    connect(passwordWindow, &PasswordWindow::chatWindowOpened, this,
            [this](ChatWindow* chatWindow) {
                connect(chatWindow, &ChatWindow::loggedOut, this, [this]() {
                    show();
                    m_usernameField->clear();
                    clearErrorMessage();
                });
                hide();
            });

    passwordWindow->exec();
}

void LoginWindow::onRegisterClicked()
{
    auto* registerWindow = new RegisterWindow(m_dbManager, this);
    registerWindow->exec();
}