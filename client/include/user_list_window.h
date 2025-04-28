#pragma once
#include <memory>
#include <vector>

#include <QDialog>
#include <QListWidget>
#include <QString>
#include <QVBoxLayout>

class UserListWindow : public QDialog
{
    Q_OBJECT
public:
    explicit UserListWindow(QWidget* parent = nullptr);
    ~UserListWindow() override = default;

    void updateUserList(const std::vector<QString>& userList);

private:
    void setupUi();
    void applyStyles();

    std::unique_ptr<QListWidget> m_userListWidget;
};