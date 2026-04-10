#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class VerifyFilePanel : public QWidget {
    Q_OBJECT
public:
    explicit VerifyFilePanel(QWidget* parent = nullptr);

private slots:
    void onChooseFile();

private:
    QLabel* m_pathLabel{};
    QLabel* m_resultLabel{};
    bool    m_hasResult{false};
    bool    m_resultOk{false};

    void refreshStyle();
};
