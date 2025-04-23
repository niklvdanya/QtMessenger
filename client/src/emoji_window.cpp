#include "emoji_window.h"
#include <QVBoxLayout>

EmojiWindow::EmojiWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Emoji Picker");
    setFixedSize(300, 300);

    m_emojis = {
        "😊", "😂", "😍", "😎", "🥳",
        "😢", "😡", "😴", "🤓", "😇",
        "👍", "👎", "🙌", "👏", "✌️",
        "❤️", "💔", "⭐", "🔥", "💡"
    };

    setupUi();
    applyStyles();
}

void EmojiWindow::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    m_gridLayout = new QGridLayout();
    mainLayout->addLayout(m_gridLayout);
    addEmojiButtons();
}

void EmojiWindow::applyStyles() {
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QPushButton {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            font-size: 20px;
            padding: 8px;
            font-family: 'Noto Color Emoji', 'Segoe UI Emoji', 'Arial', sans-serif;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
        }
    )");

    QFont emojiFont;
    emojiFont.setFamily("Noto Color Emoji, Segoe UI Emoji, Arial");
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        auto* button = qobject_cast<QPushButton*>(m_gridLayout->itemAt(i)->widget());
        if (button) {
            button->setFont(emojiFont);
        }
    }
}

void EmojiWindow::addEmojiButtons() {
    int row = 0;
    int col = 0;
    const int maxCols = 5;

    for (const auto& emoji : m_emojis) {
        auto* button = new QPushButton(emoji, this);
        button->setFixedSize(50, 50);
        connect(button, &QPushButton::clicked, this, [this, emoji]() {
            emit emojiSelected(emoji);
            accept(); 
        });
        m_gridLayout->addWidget(button, row, col);
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}