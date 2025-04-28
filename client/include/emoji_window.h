#pragma once
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <vector>

class EmojiWindow : public QDialog {
    Q_OBJECT
public:
    explicit EmojiWindow(QWidget* parent = nullptr);
    ~EmojiWindow() override = default;

signals:
    void emojiSelected(const QString& emoji);

private:
    void setupUi();
    void applyStyles();
    void addEmojiButtons();

    std::vector<QString> m_emojis;
    QGridLayout* m_gridLayout{nullptr};
};