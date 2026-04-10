#pragma once
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "data_structures.h"
#include "rangeworker.h"

class RangeTestPanel : public QWidget {
    Q_OBJECT
public:
    explicit RangeTestPanel(QWidget* parent = nullptr);

signals:
    void resultReady(std::vector<int> speeds, bool violation,
                     int numerator, int denominator);

private slots:
    void onRun();
    void onStop();
    void onProgress(int current, int total, QString status);
    void onFinished(RangeResult result);

private:
    QSpinBox*     m_startBox{};
    QSpinBox*     m_endBox{};
    QSpinBox*     m_runnersBox{};
    QSpinBox*     m_startMaxBox{};
    QCheckBox*    m_preTestBox{};
    QRadioButton* m_geoRadio{};
    QRadioButton* m_numRadio{};
    QProgressBar* m_progress{};
    QLabel*       m_statusLabel{};
    QPushButton*  m_runBtn{};
    QPushButton*  m_stopBtn{};
    QPushButton*  m_animateBtn{};
    QLabel*       m_resultLabel{};

    std::unique_ptr<RangeWorker> m_worker;
    std::vector<int> m_lastSpeeds;
    int  m_lastNumerator{0}, m_lastDenominator{0};
    bool m_hasResult{false};
    bool m_resultOk{false};

    void setRunning(bool running);
    void refreshStyle();
};
