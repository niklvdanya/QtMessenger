#pragma once
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QString>
#include <vector>

class UserListWindow : public QDialog {
    Q_OBJECT
public:
    explicit UserListWindow(QWidget* parent = nullptr);
    void updateUserList(const std::vector<QString>& userList);

private:
    void setupUi();
    void applyStyles();

    std::unique_ptr<QListWidget> m_userListWidget;
};