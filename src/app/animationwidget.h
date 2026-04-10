#pragma once
#include <QWidget>
#include <QTimer>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <vector>
#include <cmath>

class AnimationCanvas : public QWidget {
    Q_OBJECT
public:
    explicit AnimationCanvas(QWidget* parent = nullptr);
    void setState(const std::vector<int>& speeds, double t, double lonely_time);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::vector<int> m_speeds;
    double m_t{0.0};
    double m_lonely_time{0.0};
};

class AnimationWidget : public QWidget {
    Q_OBJECT
public:
    AnimationWidget(std::vector<int> speeds, int lonely_num, int lonely_den,
                    QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onPlayPause();
    void onReset();
    void onScrubberMoved(int value);
    void onJumpToLonely();
    void onTick();
    void onSaveImage();
    void applyTheme();
private:
    std::vector<int> m_speeds;
    double  m_lonelyTime{0.0};
    double  m_t{0.0};
    double  m_speedMult{1.0};
    bool    m_playing{false};

    static constexpr int    SLIDER_STEPS   = 10000;
    static constexpr int    FRAME_MS       = 16;
    static constexpr size_t INFO_THRESHOLD = 20;

    QTimer*          m_timer{};
    AnimationCanvas* m_canvas{};
    QSlider*         m_scrubber{};
    QPushButton*     m_playBtn{};
    QLabel*          m_timeLabel{};
    QLabel*          m_infoPanel{};
    QCheckBox*       m_showInfoBox{};

    double tMax() const;
    void   updateTimeLabel();
    void   updateInfoPanel();
    void   setScrubberFromT();
};
