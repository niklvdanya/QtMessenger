#include "register_window.h"

#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

RegisterWindow::RegisterWindow(IDatabase* dbManager, QWidget* parent)
    : QDialog(parent), m_dbManager(dbManager)
{
    setWindowTitle("Register");
    setFixedSize(300, 250);
    setupUi();
    applyStyles();
}

void RegisterWindow::setupUi()
{
    setWindowTitle("Create Account");
    setFixedSize(400, 550);

    auto* mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");
    mainContainer->setFixedWidth(350);

    auto* titleLabel = new QLabel("Create Account", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* usernameContainer = new QWidget(this);
    auto* usernameLayout = new QHBoxLayout(usernameContainer);
    usernameLayout->setContentsMargins(0, 0, 0, 0);
    usernameLayout->setSpacing(10);

    auto* usernameIcon = new QLabel("👤", this);
    usernameIcon->setFixedSize(30, 30);
    usernameIcon->setAlignment(Qt::AlignCenter);
    usernameIcon->setObjectName("usernameIcon");

    m_usernameField = std::make_unique<QLineEdit>(this);
    m_usernameField->setPlaceholderText("Choose a username");
    m_usernameField->setMinimumHeight(50);

    usernameLayout->addWidget(usernameIcon);
    usernameLayout->addWidget(m_usernameField.get());

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
    m_passwordField->setPlaceholderText("Choose a password");
    m_passwordField->setMinimumHeight(50);

    passwordLayout->addWidget(passwordIcon);
    passwordLayout->addWidget(m_passwordField.get());

    m_statusLabel = std::make_unique<QLabel>("Fill in all fields", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("statusLabel");

    m_registerButton = std::make_unique<QPushButton>("Register", this);
    m_registerButton->setMinimumHeight(50);
    m_registerButton->setCursor(Qt::PointingHandCursor);

    auto* cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumHeight(50);
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setObjectName("cancelButton");

    auto* containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(25, 25, 25, 25);

    containerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    containerLayout->addSpacing(20);
    containerLayout->addWidget(usernameContainer);
    containerLayout->addSpacing(10);
    containerLayout->addWidget(passwordContainer);
    containerLayout->addSpacing(20);
    containerLayout->addWidget(m_statusLabel.get(), 0, Qt::AlignCenter);
    containerLayout->addSpacing(10);
    containerLayout->addWidget(m_registerButton.get());
    containerLayout->addWidget(cancelButton);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(mainContainer, 0, Qt::AlignCenter);

    connect(m_registerButton.get(), &QPushButton::clicked, this,
            &RegisterWindow::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    applyStyles();
}

void RegisterWindow::applyStyles()
{
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
            font-size: 24px;
            font-weight: bold;
            color: #179cde;
            margin-bottom: 10px;
        )");
    }

    QLabel* usernameIcon = findChild<QLabel*>("usernameIcon");
    if (usernameIcon) {
        usernameIcon->setStyleSheet(R"(
            font-size: 18px; 
            color: #179cde;
        )");
    }

    QLabel* passwordIcon = findChild<QLabel*>("passwordIcon");
    if (passwordIcon) {
        passwordIcon->setStyleSheet(R"(
            font-size: 18px; 
            color: #179cde;
        )");
    }

    if (m_statusLabel) {
        m_statusLabel->setStyleSheet(R"(
            font-size: 14px;
            color: #737373;
        )");
    }

    if (m_usernameField && m_passwordField) {
        QString inputStyle = R"(
            border: 1px solid #e0e0e0;
            border-radius: 25px;
            padding: 12px 20px;
            font-size: 14px;
            background-color: #f5f7fa;
        )";
        m_usernameField->setStyleSheet(inputStyle);
        m_passwordField->setStyleSheet(inputStyle);
    }

    if (m_registerButton) {
        m_registerButton->setStyleSheet(R"(
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

void RegisterWindow::showMessage(const QString& message, bool isError)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        if (isError) {
            m_statusLabel->setStyleSheet(R"(
                font-size: 14px;
                color: #dc3545;
            )");
        } else {
            m_statusLabel->setStyleSheet(R"(
                font-size: 14px;
                color: #28a745;
            )");
        }
    }
}

void RegisterWindow::onRegisterClicked()
{
    QString username = m_usernameField->text().trimmed();
    QString password = m_passwordField->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        showMessage("Please fill in all fields", true);
        return;
    }

    if (m_dbManager->userExists(username)) {
        showMessage("Username already exists", true);
        return;
    }

    if (m_dbManager->addUser(username, password)) {
        showMessage("Registration successful!", false);
        QTimer::singleShot(1000, this, &QDialog::accept);
    } else {
        showMessage("Registration failed", true);
    }
}