#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <vector>

class ManualTestPanel : public QWidget {
    Q_OBJECT
public:
    explicit ManualTestPanel(QWidget* parent = nullptr);
    void setSpeedText(const QString& text);

signals:
    void resultReady(std::vector<int> speeds, bool valid, int numerator, int denominator);

private slots:
    void onRun();
    void onImport();
    void onAnimate();

private:
    QLineEdit*    m_speedsInput{};
    QCheckBox*    m_preTestBox{};
    QCheckBox*    m_findMaxBox{};
    QRadioButton* m_geoRadio{};
    QRadioButton* m_numRadio{};
    QRadioButton* m_primeRadio{};
    QLabel*       m_resultLabel{};
    QPushButton*  m_animateBtn{};

    std::vector<int> m_lastSpeeds;
    int  m_lastNumerator{0}, m_lastDenominator{0};
    bool m_hasResult{false};
    bool m_resultOk{false};

    std::vector<int> parseSpeeds() const;
    void showResult(const QString& text, bool ok);
    void refreshStyle();
};
