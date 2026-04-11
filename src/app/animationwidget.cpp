#include "animationwidget.h"
#include "thememanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSignalBlocker>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// AnimationCanvas
// ---------------------------------------------------------------------------

AnimationCanvas::AnimationCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(300, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void AnimationCanvas::setState(const std::vector<int>& speeds, double t, double lonely_time) {
    m_speeds = speeds; m_t = t; m_lonely_time = lonely_time;
    update();
}

void AnimationCanvas::paintEvent(QPaintEvent*) {
    const auto& c = ThemeManager::instance().colors();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const double cx = width() / 2.0, cy = height() / 2.0;
    const double radius = std::min(width(), height()) * 0.40;

    // Background
    p.fillRect(rect(), c.canvasBg);
    if (m_speeds.empty()) return;

    const int    n         = static_cast<int>(m_speeds.size());
    const double threshold = 1.0 / (n + 1);

    // Lonely zone sector (shaded arc around the origin)
    QColor zoneCol = c.ok;
    zoneCol.setAlphaF(c.zoneAlpha);
    auto drawZone = [&](double sign) {
        double deg = threshold * 360.0;
        QPainterPath pp;
        pp.moveTo(cx, cy);
        pp.arcTo(QRectF(cx-radius, cy-radius, 2*radius, 2*radius),
                 sign * (-deg/2.0), sign * deg);
        pp.closeSubpath();
        p.fillPath(pp, zoneCol);
    };
    drawZone(1.0); drawZone(-1.0);

    // Track ring
    p.setPen(QPen(c.track, 2)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy), radius, radius);

    // Origin marker
    p.setPen(Qt::NoPen); p.setBrush(c.originDot);
    p.drawEllipse(QPointF(cx + radius, cy), 5.0, 5.0);

    // Runner dots
    const auto& palette = c.runners;
    for (int i = 0; i < n; ++i) {
        double pos = std::fmod(m_speeds[i] * m_t, 1.0);
        if (pos < 0) pos += 1.0;
        double angle = pos * 2.0 * M_PI;
        double rx = cx + radius * std::cos(-angle);
        double ry = cy + radius * std::sin(-angle);
        double dist  = std::min(pos, 1.0 - pos);
        bool   lonely = dist >= threshold;
        QColor col    = palette[i % static_cast<int>(palette.size())];

        if (lonely) {
            QColor ring = col; ring.setAlphaF(0.45);
            p.setPen(QPen(ring, 2.5)); p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(rx, ry), 11.0, 11.0);
        }
        p.setPen(Qt::NoPen); p.setBrush(col);
        p.drawEllipse(QPointF(rx, ry), 7.0, 7.0);

        // Speed label radiating outward from dot (only for small runner counts)
        if (n <= 15) {
            p.setPen(col);
            p.setFont(QFont("sans-serif", 7, QFont::Bold));
            double lx = cx + (radius + 14) * std::cos(-angle) - 8;
            double ly = cy + (radius + 14) * std::sin(-angle) - 6;
            p.drawText(QRectF(lx, ly, 30, 14), Qt::AlignCenter,
                       QString::number(m_speeds[i]));
        }
    }
}

// ---------------------------------------------------------------------------
// AnimationWidget
// ---------------------------------------------------------------------------

// Logarithmic speed mapping: speed = 2^(-4 + 5*v/1000)
// v=0 → 0.0625×, v=800 → 1.0×, v=1000 → 2.0×
static double sliderToSpeed(int v) {
    return std::pow(2.0, -4.0 + 5.0 * v / 1000.0);
}
static int speedToSlider(double s) {
    return static_cast<int>(std::round((std::log2(s) + 4.0) / 5.0 * 1000.0));
}

AnimationWidget::AnimationWidget(std::vector<int> speeds, int lonely_num, int lonely_den,
                                 QWidget* parent)
    : QWidget(parent, Qt::Window),
      m_speeds(std::move(speeds)),
      m_lonelyTime(lonely_den > 0 ? double(lonely_num) / lonely_den : 0.0)
{
    setWindowTitle("Animation - Lonely Runner");
    resize(720, 540);
    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0,0,0,0); vbox->setSpacing(0);

    m_canvas    = new AnimationCanvas(this);
    m_infoPanel = new QLabel;
    m_infoPanel->setFixedWidth(175);
    m_infoPanel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto* canvasRow = new QHBoxLayout;
    canvasRow->addWidget(m_canvas, 1);
    canvasRow->addWidget(m_infoPanel);
    vbox->addLayout(canvasRow, 1);

    if (m_speeds.size() > INFO_THRESHOLD) {
        auto* warn = new QLabel("High runner count \xe2\x80\x94 detail hidden for clarity.");
        m_showInfoBox = new QCheckBox("Show info anyway");
        auto* warnRow = new QHBoxLayout;
        warnRow->addWidget(warn); warnRow->addWidget(m_showInfoBox);
        vbox->addLayout(warnRow);
        m_infoPanel->setVisible(false);
        connect(m_showInfoBox, &QCheckBox::toggled, m_infoPanel, &QLabel::setVisible);
    }

    m_scrubber = new QSlider(Qt::Horizontal);
    m_scrubber->setRange(0, SLIDER_STEPS);
    vbox->addWidget(m_scrubber);

    // Control bar
    auto* bar       = new QWidget;
    auto* barLayout = new QHBoxLayout(bar);
    barLayout->setContentsMargins(8,6,8,6);
    m_playBtn      = new QPushButton("Play");
    auto* resetBtn = new QPushButton("Reset");
    auto* jumpBtn  = new QPushButton("Jump to t");
    jumpBtn->setEnabled(m_lonelyTime > 0.0);
    barLayout->addWidget(m_playBtn);
    barLayout->addWidget(resetBtn);
    barLayout->addWidget(jumpBtn);
    barLayout->addSpacing(12);
    barLayout->addWidget(new QLabel("Speed:"));

    m_speedValueLabel = new QLabel("1.00\xc3\x97");  // UTF-8 "×"
    m_speedValueLabel->setMinimumWidth(48);
    m_speedValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    barLayout->addWidget(m_speedValueLabel);

    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(0, 1000);
    m_speedSlider->setValue(speedToSlider(1.0));  // default 1×
    m_speedSlider->setFixedWidth(120);
    m_speedSlider->setToolTip("Speed: drag left for slower, right for faster");
    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int v) {
        m_speedMult = sliderToSpeed(v);
        m_speedValueLabel->setText(
            QString::number(m_speedMult, 'g', 2) + "\xc3\x97");
    });
    barLayout->addWidget(m_speedSlider);
    barLayout->addStretch();

    m_timeLabel = new QLabel("t = 0.0000");
    barLayout->addWidget(m_timeLabel);

    auto* saveBtn = new QPushButton("Save PNG");
    barLayout->addWidget(saveBtn);
    vbox->addWidget(bar);

    // Timer
    m_timer = new QTimer(this);
    m_timer->setInterval(FRAME_MS);
    connect(m_timer,    &QTimer::timeout,       this, &AnimationWidget::onTick);
    connect(m_playBtn,  &QPushButton::clicked,  this, &AnimationWidget::onPlayPause);
    connect(resetBtn,   &QPushButton::clicked,  this, &AnimationWidget::onReset);
    connect(jumpBtn,    &QPushButton::clicked,  this, &AnimationWidget::onJumpToLonely);
    connect(m_scrubber, &QSlider::valueChanged, this, &AnimationWidget::onScrubberMoved);
    connect(saveBtn,    &QPushButton::clicked,  this, &AnimationWidget::onSaveImage);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &AnimationWidget::applyTheme);

    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    applyTheme();
    updateTimeLabel();
    updateInfoPanel();
}

double AnimationWidget::tMax() const { return std::max(1.0, m_lonelyTime * 2.5); }

void AnimationWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    ThemeManager::instance().applyTitleBarHint(windowHandle());
}

void AnimationWidget::applyTheme() {
    const auto& c = ThemeManager::instance().colors();
    m_infoPanel->setStyleSheet(
        QString("QLabel{background:%1;color:%2;padding:10px;font-family:monospace;}")
        .arg(c.base.name(), c.muted.name()));
    m_timeLabel->setStyleSheet(
        QString("color:%1;").arg(c.highlight.name()));
    m_canvas->update();
    updateInfoPanel();
}

void AnimationWidget::onTick() {
    m_t += m_speedMult * FRAME_MS / 1000.0;
    if (m_t > tMax()) m_t = 0.0;
    setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}
void AnimationWidget::onPlayPause() {
    m_playing = !m_playing;
    m_playBtn->setText(m_playing ? "Pause" : "Play");
    m_playing ? m_timer->start() : m_timer->stop();
}
void AnimationWidget::onReset() {
    m_timer->stop(); m_playing = false;
    m_playBtn->setText("Play");
    m_t = 0.0; setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}
void AnimationWidget::onJumpToLonely() {
    m_t = m_lonelyTime; setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}
void AnimationWidget::onScrubberMoved(int value) {
    if (m_timer->isActive()) return;
    m_t = tMax() * double(value) / SLIDER_STEPS;
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}
void AnimationWidget::setScrubberFromT() {
    int v = static_cast<int>(m_t / tMax() * SLIDER_STEPS);
    QSignalBlocker blocker(m_scrubber);
    m_scrubber->setValue(std::clamp(v, 0, SLIDER_STEPS));
}
void AnimationWidget::onSaveImage() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save image", "animation.png", "PNG (*.png)");
    if (!path.isEmpty()) m_canvas->grab().save(path);
}
void AnimationWidget::updateTimeLabel() {
    m_timeLabel->setText(QString("t = %1").arg(m_t, 0, 'f', 4));
}
void AnimationWidget::updateInfoPanel() {
    if (!m_infoPanel->isVisible()) return;
    const auto&  c         = ThemeManager::instance().colors();
    const int    n         = static_cast<int>(m_speeds.size());
    const double threshold = 1.0 / (n + 1);
    QString text;
    text += QString("<b>t = %1</b><br>").arg(m_t, 0, 'f', 5);
    if (m_lonelyTime > 0.0)
        text += QString("Lonely t \xe2\x89\x88 %1<br>").arg(m_lonelyTime, 0, 'f', 5);
    text += QString("Threshold: 1/%1 = %2<br>").arg(n + 1).arg(threshold, 0, 'f', 4);
    text += "<br><b>Runners:</b><br>";
    text += "<table cellpadding='2' cellspacing='0'>";
    for (int i = 0; i < n; ++i) {
        double pos  = std::fmod(m_speeds[i] * m_t, 1.0);
        if (pos < 0) pos += 1.0;
        double dist = std::min(pos, 1.0 - pos);
        bool lonely = dist >= threshold;
        QString col = c.runners[i % static_cast<int>(c.runners.size())].name();
        QString statusCol = lonely ? c.ok.name() : c.fail.name();
        QString statusSym = lonely ? "\xe2\x9c\x93" : "\xe2\x9c\x97";
        text += QString(
            "<tr>"
            "<td><span style='color:%1;font-size:14px'>\xe2\xac\xa4</span></td>"
            "<td>v=%2</td>"
            "<td>%3</td>"
            "<td style='color:%4'>%5</td>"
            "</tr>")
            .arg(col)
            .arg(m_speeds[i])
            .arg(dist, 0, 'f', 4)
            .arg(statusCol, statusSym);
    }
    text += "</table>";
    m_infoPanel->setText(text);
}
