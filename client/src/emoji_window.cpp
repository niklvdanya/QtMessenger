#include "emoji_window.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>

EmojiWindow::EmojiWindow(QWidget* parent) : QDialog(parent) {
    setupUi();
    applyStyles();
}

void EmojiWindow::setupUi() {
    setWindowTitle("Select Emoji");
    setFixedSize(450, 550);
    
    auto* titleLabel = new QLabel("Emojis", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(50);
    auto* tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("tabWidget");
    
    auto* smilesTab = new QWidget();
    auto* objectsTab = new QWidget();
    auto* animalsTab = new QWidget();
    
    auto* smileyLayout = new QGridLayout(smilesTab);
    auto* objectsLayout = new QGridLayout(objectsTab);
    auto* animalsLayout = new QGridLayout(animalsTab);
    
    smileyLayout->setSpacing(10);
    objectsLayout->setSpacing(10);
    animalsLayout->setSpacing(10);
    
    std::vector<QString> smileys = {
        "😊", "😂", "😍", "😎", "🥳",  
        "😢", "😡", "😴", "🤓", "😇", 
        "😉", "🤔", "😏", "😋", "🤗",  
        "😄", "😅", "🤣", "😘", "🙂",  
        "😗", "🤑", "🤪", "😜", "😝"   
    };
    
    std::vector<QString> objects = {
        "💻", "📱", "⌚", "📷", "🔋",  
        "💡", "🔍", "🔑", "📁", "✏️",  
        "📚", "🎮", "🎧", "🎬", "🎵",  
        "🚗", "✈️", "🚀", "⚽", "🏆",  
        "💰", "💎", "🎁", "🔥", "❤️"   
    };
    
    std::vector<QString> animals = {
        "🐶", "🐱", "🐭", "🐹", "🐰",  
        "🦊", "🐻", "🐼", "🐨", "🐯",  
        "🦁", "🐮", "🐷", "🐸", "🐵",  
        "🐔", "🐧", "🐦", "🦆", "🦅",  
        "🐝", "🦋", "🐙", "🦄", "🐠"   
    };

    addEmojiButtonsToGrid(smileys, smileyLayout, 5);
    
    addEmojiButtonsToGrid(objects, objectsLayout, 5);
    
    addEmojiButtonsToGrid(animals, animalsLayout, 5);
    
    tabWidget->addTab(smilesTab, "😊 Smileys");
    tabWidget->addTab(objectsTab, "🎮 Objects");
    tabWidget->addTab(animalsTab, "🐱 Animals");
    
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(tabWidget);
    
    applyStyles();
}

void EmojiWindow::addEmojiButtonsToGrid(const std::vector<QString>& emojis, QGridLayout* grid, int columns) {
    int row = 0;
    int col = 0;
    
    for (const auto& emoji : emojis) {
        auto* button = new QPushButton(emoji, this);
        button->setFixedSize(60, 60);
        button->setObjectName("emojiButton");
        
        QFont emojiFont("Segoe UI Emoji, Apple Color Emoji, Noto Color Emoji");
        emojiFont.setPointSize(24);
        button->setFont(emojiFont);
        
        connect(button, &QPushButton::clicked, this, [this, emoji]() {
            emit emojiSelected(emoji);
            accept();
        });
        
        grid->addWidget(button, row, col);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
}

void EmojiWindow::applyStyles() {
    setStyleSheet(R"(
        QDialog {
            background-color: white;
            border-radius: 10px;
        }
        
        QLabel#titleLabel {
            font-size: 22px;
            font-weight: bold;
            color: #333333;
            margin: 10px 0;
        }
        
        QTabWidget::pane {
            border: 1px solid #e0e0e0;
            border-radius: 5px;
            background-color: white;
            top: -1px;
        }
        
        QTabBar::tab {
            background-color: #f5f5f5;
            padding: 8px 15px;
            margin-right: 2px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            font-size: 14px;
        }
        
        QTabBar::tab:selected {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-bottom-color: white;
        }
        
        QTabBar::tab:hover:!selected {
            background-color: #e0e0e0;
        }
        
        QPushButton#emojiButton {
            background-color: white;
            border: 1px solid #f0f0f0;
            border-radius: 8px;
            text-align: center;
        }
        
        QPushButton#emojiButton:hover {
            background-color: #f0f2f5;
            border: 1px solid #e0e0e0;
        }
        
        QPushButton#emojiButton:pressed {
            background-color: #e0e0e0;
        }
    )");
}