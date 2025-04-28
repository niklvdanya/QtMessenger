#include "password_window.h"
#include "network_client_factory.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

PasswordWindow::PasswordWindow(const QString& username, DatabaseManager* dbManager, QWidget* parent)
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

void PasswordWindow::onSubmitClicked() {
    QString password = m_passwordField->text().trimmed();
    if (password.isEmpty()) {
        m_statusLabel->setText("Please enter a password");
        m_statusLabel->setStyleSheet("color: #dc3545;");
        return;
    }

    // Проверяем локально, чтобы убедиться, что учетные данные действительны
    bool localCheck = m_dbManager->checkUser(m_username, password);
    qDebug() << "Local credential check for user:" << m_username 
             << "result:" << (localCheck ? "valid" : "invalid");

    if (localCheck) {
        // Создаем сетевого клиента и подключаемся к серверу
        auto networkClient = NetworkClientFactory::createTcpClient(nullptr);
        
        // Подробно логируем процесс подключения
        qDebug() << "Connecting to server with username:" << m_username;
                 
        networkClient->connectToServer("127.0.0.1", 12345, m_username.toStdString(), password.toStdString()); 
        
        // Создаем и показываем окно чата
        ChatWindow* chatWindow = new ChatWindow(std::move(networkClient));
        chatWindow->setAttribute(Qt::WA_DeleteOnClose);  // Автоматически удалять окно при закрытии
        chatWindow->show();
        
        // Отправляем сигнал о создании окна чата
        emit chatWindowOpened(chatWindow);
        
        accept();  // Закрываем диалог ввода пароля
    } else {
        m_statusLabel->setText("Incorrect password");
        m_statusLabel->setStyleSheet("color: #dc3545;");
    }
}