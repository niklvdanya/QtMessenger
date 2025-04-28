#include "register_window.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QTimer>

RegisterWindow::RegisterWindow(IDatabase* dbManager, QWidget* parent)
    : QDialog(parent), m_dbManager(dbManager) {
    setWindowTitle("Register");
    setFixedSize(300, 250);
    setupUi();
    applyStyles();
}

void RegisterWindow::setupUi() {
    m_statusLabel = std::make_unique<QLabel>("Create a new account", this);
    m_usernameField = std::make_unique<QLineEdit>(this);
    m_passwordField = std::make_unique<QLineEdit>(this);
    m_registerButton = std::make_unique<QPushButton>("Register", this);

    m_usernameField->setPlaceholderText("Enter username...");
    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setPlaceholderText("Enter password...");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_statusLabel.get(), 0, Qt::AlignCenter);
    mainLayout->addWidget(m_usernameField.get());
    mainLayout->addWidget(m_passwordField.get());
    mainLayout->addWidget(m_registerButton.get());

    connect(m_registerButton.get(), &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
}

void RegisterWindow::applyStyles() {
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLabel {
            font-size: 16px;
            color: #333333;
            margin-bottom: 10px;
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

void RegisterWindow::showMessage(const QString& message, bool isError) {
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(isError ? "color: #dc3545;" : "color: #28a745;");
}

void RegisterWindow::onRegisterClicked() {
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