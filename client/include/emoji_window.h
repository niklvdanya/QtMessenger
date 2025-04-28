#pragma once
#include <vector>

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>

class EmojiWindow : public QDialog
{
    Q_OBJECT
public:
    explicit EmojiWindow(QWidget* parent = nullptr);
    ~EmojiWindow() override = default;

signals:
    void emojiSelected(const QString& emoji);

private:
    void setupUi();
    void applyStyles();
    void addEmojiButtonsToGrid(const std::vector<QString>& emojis, QGridLayout* grid, int columns);

    std::vector<QString> m_emojis;
    QGridLayout* m_gridLayout{nullptr};
};