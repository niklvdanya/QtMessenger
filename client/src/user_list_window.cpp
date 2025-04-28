#include "user_list_window.h"
#include <QDebug>

UserListWindow::UserListWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Online Users");
    setFixedSize(250, 300);
    setupUi();
    applyStyles();
}

void UserListWindow::setupUi() {
    m_userListWidget = std::make_unique<QListWidget>(this);
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_userListWidget.get());
}

void UserListWindow::applyStyles() {
    setStyleSheet(R"(
        QDialog {
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
    )");
}

void UserListWindow::updateUserList(const std::vector<QString>& userList) {
    m_userListWidget->clear();
    for (const auto& username : userList) {
        m_userListWidget->addItem(username);
    }
}