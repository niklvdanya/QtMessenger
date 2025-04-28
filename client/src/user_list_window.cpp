#include "user_list_window.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>

UserListWindow::UserListWindow(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Online Users");
    setFixedSize(250, 300);
    setupUi();
    applyStyles();
}

void UserListWindow::setupUi()
{
    setWindowTitle("Online Users");
    setFixedSize(350, 600);

    auto* titleLabel = new QLabel("Online Users", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(50);

    auto* searchContainer = new QWidget(this);
    auto* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(10, 0, 10, 0);
    searchLayout->setSpacing(10);

    auto* searchIcon = new QLabel("🔍", this);
    searchIcon->setFixedSize(20, 20);
    searchIcon->setObjectName("searchIcon");

    auto* searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search users...");
    searchField->setObjectName("searchField");
    searchField->setMinimumHeight(40);

    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(searchField);

    auto* counterLabel = new QLabel("0 users", this);
    counterLabel->setObjectName("counterLabel");
    counterLabel->setAlignment(Qt::AlignCenter);
    counterLabel->setFixedHeight(30);

    m_userListWidget = std::make_unique<QListWidget>(this);
    m_userListWidget->setObjectName("userList");
    m_userListWidget->setFrameShape(QFrame::NoFrame);
    m_userListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_userListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(searchContainer);
    mainLayout->addWidget(counterLabel);
    mainLayout->addWidget(m_userListWidget.get());

    connect(searchField, &QLineEdit::textChanged, this, [this, searchField, counterLabel]() {
        QString searchText = searchField->text().toLower();
        int visibleCount = 0;

        for (int i = 0; i < m_userListWidget->count(); ++i) {
            QListWidgetItem* item = m_userListWidget->item(i);
            QWidget* widget = m_userListWidget->itemWidget(item);

            if (widget) {
                QLabel* nameLabel = widget->findChild<QLabel*>("nameLabel");
                if (nameLabel) {
                    bool visible = nameLabel->text().toLower().contains(searchText);
                    item->setHidden(!visible);

                    if (visible) {
                        visibleCount++;
                    }
                }
            }
        }

        counterLabel->setText(
            QString("%1 %2").arg(visibleCount).arg(visibleCount == 1 ? "user" : "users"));
    });

    applyStyles();
}

void UserListWindow::applyStyles()
{
    setStyleSheet(R"(
        QDialog {
            background-color: white;
            border-radius: 10px;
        }
        
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: bold;
            color: #333333;
            margin: 10px 0;
        }
        
        QLabel#counterLabel {
            font-size: 14px;
            color: #737373;
            margin: 5px 0;
        }
        
        QLabel#searchIcon {
            color: #737373;
            font-size: 16px;
        }
        
        QLineEdit#searchField {
            border: 1px solid #e0e0e0;
            border-radius: 20px;
            padding: 5px 15px;
            background-color: #f5f7fa;
            font-size: 14px;
            margin: 5px 0;
        }
        
        QLineEdit#searchField:focus {
            border: 1.5px solid #179cde;
        }
        
        QListWidget#userList {
            background-color: white;
            border: none;
            outline: none;
            padding: 5px;
        }
        
        QListWidget#userList::item {
            border-bottom: 1px solid #f0f0f0;
            padding: 3px 0;
        }
        
        QListWidget#userList::item:selected {
            background-color: transparent;
        }
        
        QListWidget#userList::item:hover {
            background-color: #f8f9fa;
        }
    )");
}

void UserListWindow::updateUserList(const std::vector<QString>& userList)
{
    m_userListWidget->clear();

    QLabel* counterLabel = findChild<QLabel*>("counterLabel");
    if (counterLabel) {
        counterLabel->setText(
            QString("%1 %2").arg(userList.size()).arg(userList.size() == 1 ? "user" : "users"));
    }

    for (const auto& username : userList) {
        auto* userWidget = new QWidget(m_userListWidget.get());
        auto* userLayout = new QHBoxLayout(userWidget);
        userLayout->setContentsMargins(5, 10, 5, 10);
        userLayout->setSpacing(10);

        auto* avatarLabel = new QLabel(userWidget);
        avatarLabel->setFixedSize(50, 50);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setObjectName("avatarLabel");

        QChar firstLetter = username.at(0).toUpper();
        avatarLabel->setText(QString(firstLetter));

        avatarLabel->setStyleSheet(R"(
            background-color: #179cde;
            color: white;
            border-radius: 25px;
            font-weight: bold;
            font-size: 22px;
        )");

        auto* infoWidget = new QWidget(userWidget);
        auto* infoLayout = new QVBoxLayout(infoWidget);
        infoLayout->setContentsMargins(0, 0, 0, 0);
        infoLayout->setSpacing(2);

        auto* nameLabel = new QLabel(username, infoWidget);
        nameLabel->setObjectName("nameLabel");
        nameLabel->setStyleSheet(R"(
            font-size: 16px;
            font-weight: bold;
            color: #333333;
        )");

        auto* statusLabel = new QLabel("online", infoWidget);
        statusLabel->setStyleSheet(R"(
            font-size: 14px;
            color: #179cde;
        )");

        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(statusLabel);

        auto* chatButton = new QPushButton(userWidget);
        chatButton->setText("💬");
        chatButton->setFixedSize(40, 40);
        chatButton->setCursor(Qt::PointingHandCursor);
        chatButton->setObjectName("chatButton");
        chatButton->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                border: none;
                font-size: 18px;
                color: #737373;
            }
            QPushButton:hover {
                background-color: #f0f2f5;
                border-radius: 20px;
            }
        )");

        userLayout->addWidget(avatarLabel);
        userLayout->addWidget(infoWidget, 1);
        userLayout->addWidget(chatButton);

        auto* item = new QListWidgetItem(m_userListWidget.get());
        item->setSizeHint(QSize(m_userListWidget->width() - 40, 70));
        m_userListWidget->addItem(item);
        m_userListWidget->setItemWidget(item, userWidget);
    }
}