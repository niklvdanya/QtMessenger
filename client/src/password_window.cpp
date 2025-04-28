#include "password_window.h"
#include "network_client_factory.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

PasswordWindow::PasswordWindow(const QString& username, IDatabase* dbManager, QWidget* parent)
    : QDialog(parent), m_username(username), m_dbManager(dbManager) {
    setWindowTitle("Enter Password");
    setFixedSize(300, 200);
    setupUi();
    applyStyles();
}

void PasswordWindow::setupUi() {
    setWindowTitle("Login");
    setFixedSize(400, 550);

    auto* mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");
    mainContainer->setFixedWidth(350);

    m_statusLabel = std::make_unique<QLabel>(QString("Login for %1").arg(m_username), this);
    m_statusLabel->setObjectName("titleLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    
    auto* avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(100, 100);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setObjectName("avatarLabel");

    QChar firstLetter = m_username.at(0).toUpper();
    avatarLabel->setText(QString(firstLetter));
 
    auto* passwordContainer = new QWidget(this);
    auto* passwordLayout = new QHBoxLayout(passwordContainer);
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(10);
    
    auto* passwordIcon = new QLabel("🔒", this);
    passwordIcon->setFixedSize(30, 30);
    passwordIcon->setAlignment(Qt::AlignCenter);
    passwordIcon->setObjectName("passwordIcon");
    
    m_passwordField = std::make_unique<QLineEdit>(this);
    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setPlaceholderText("Enter password");
    m_passwordField->setMinimumHeight(50);
    
    passwordLayout->addWidget(passwordIcon);
    passwordLayout->addWidget(m_passwordField.get());

    auto* errorLabel = new QLabel(this);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setVisible(false); 

    m_submitButton = std::make_unique<QPushButton>("Login", this);
    m_submitButton->setMinimumHeight(50);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    
    auto* cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumHeight(50);
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setObjectName("cancelButton");

    auto* containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(25, 25, 25, 25);
    
    containerLayout->addWidget(m_statusLabel.get(), 0, Qt::AlignCenter);
    containerLayout->addSpacing(10);
    containerLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
    containerLayout->addSpacing(20);
    containerLayout->addWidget(passwordContainer);
    containerLayout->addWidget(errorLabel);
    containerLayout->addSpacing(10);
    containerLayout->addWidget(m_submitButton.get());
    containerLayout->addWidget(cancelButton);
    
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(mainContainer, 0, Qt::AlignCenter);

    connect(m_submitButton.get(), &QPushButton::clicked, this, &PasswordWindow::onSubmitClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordField.get(), &QLineEdit::returnPressed, this, &PasswordWindow::onSubmitClicked);

    applyStyles();
}

void PasswordWindow::applyStyles() {
    setStyleSheet("QDialog { background-color: #f0f2f5; }");

    QWidget* mainContainer = findChild<QWidget*>("mainContainer");
    if (mainContainer) {
        mainContainer->setStyleSheet(R"(
            background-color: white;
            border-radius: 30px;
        )");
    }
    
    QLabel* titleLabel = findChild<QLabel*>("titleLabel");
    if (titleLabel) {
        titleLabel->setStyleSheet(R"(
            font-size: 22px;
            font-weight: bold;
            color: #333333;
            margin-bottom: 10px;
        )");
    }
    
    QLabel* avatarLabel = findChild<QLabel*>("avatarLabel");
    if (avatarLabel) {
        avatarLabel->setStyleSheet(R"(
            background-color: #179cde;
            color: white;
            border-radius: 50px;
            font-weight: bold;
            font-size: 40px;
        )");
    }
    

    QLabel* passwordIcon = findChild<QLabel*>("passwordIcon");
    if (passwordIcon) {
        passwordIcon->setStyleSheet(R"(
            font-size: 18px; 
            color: #179cde;
        )");
    }
    

    QLabel* errorLabel = findChild<QLabel*>("errorLabel");
    if (errorLabel) {
        errorLabel->setStyleSheet(R"(
            color: #dc3545;
            font-size: 14px;
            margin-top: 10px;
        )");
    }
    

    if (m_passwordField) {
        m_passwordField->setStyleSheet(R"(
            border: 1px solid #e0e0e0;
            border-radius: 25px;
            padding: 12px 20px;
            font-size: 14px;
            background-color: #f5f7fa;
        )");
    }
    
    if (m_submitButton) {
        m_submitButton->setStyleSheet(R"(
            background-color: #179cde;
            color: white;
            border: none;
            border-radius: 25px;
            font-size: 16px;
            font-weight: bold;
            margin: 5px 0;
        )");
    }

    QPushButton* cancelButton = findChild<QPushButton*>("cancelButton");
    if (cancelButton) {
        cancelButton->setStyleSheet(R"(
            background-color: transparent;
            color: #737373;
            border: 1px solid #d0d0d0;
            border-radius: 25px;
            font-size: 16px;
            margin: 5px 0;
        )");
    }
}

void PasswordWindow::showError(const QString& message) {
    QLabel* errorLabel = findChild<QLabel*>("errorLabel");
    if (errorLabel) {
        errorLabel->setText(message);
        errorLabel->setVisible(true);
    }
}

void PasswordWindow::onSubmitClicked() {
    QString password = m_passwordField->text().trimmed();
    if (password.isEmpty()) {
        showError("Please enter a password");
        return;
    }
    bool validCredentials = m_dbManager->checkUser(m_username, password);
    qDebug() << "Local credential check for user:" << m_username 
             << "result:" << (validCredentials ? "valid" : "invalid");

    if (validCredentials) {
        auto networkClient = NetworkClientFactory::createTcpClient(nullptr);
        if (!networkClient) {
            showError("Failed to create network client");
            return;
        }
        
        qDebug() << "Creating chat window with username:" << m_username;

        auto* chatWindow = new ChatWindow(std::move(networkClient));
        chatWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(chatWindow, &ChatWindow::loggedOut, [this]() {
            QDialog::accept();
        });
        if (auto* controller = dynamic_cast<ChatController*>(chatWindow->getController())) {
            controller->setUsername(m_username.toStdString());
            controller->connectToServer("127.0.0.1", 12345, 
                                     m_username.toStdString(), 
                                     password.toStdString());
        }
        
        emit chatWindowOpened(chatWindow);
        chatWindow->show();
        
        accept();  
    } else {
        showError("Incorrect password");
    }
}