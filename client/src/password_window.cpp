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
    m_statusLabel = std::make_unique<QLabel>("Enter your password", this);
    m_passwordField = std::make_unique<QLineEdit>(this);
    m_submitButton = std::make_unique<QPushButton>("Submit", this);

    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setPlaceholderText("Password...");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_statusLabel.get(), 0, Qt::AlignCenter);
    mainLayout->addWidget(m_passwordField.get());
    mainLayout->addWidget(m_submitButton.get());

    connect(m_submitButton.get(), &QPushButton::clicked, this, &PasswordWindow::onSubmitClicked);
    connect(m_passwordField.get(), &QLineEdit::returnPressed, this, &PasswordWindow::onSubmitClicked);
}

void PasswordWindow::applyStyles() {
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

void PasswordWindow::showError(const QString& message) {
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("color: #dc3545;");
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
        chatWindow->setAttribute(Qt::WA_DeleteOnClose)
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