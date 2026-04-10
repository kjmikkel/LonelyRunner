#include "rangetestpanel.h"
#include "animationwidget.h"
#include "numerical.h"
#include "util.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>

RangeTestPanel::RangeTestPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* cfgGroup  = new QGroupBox("Configuration");
    auto* cfgLayout = new QFormLayout(cfgGroup);
    m_startBox    = new QSpinBox; m_startBox->setRange(1,1000000);    m_startBox->setValue(1);
    m_endBox      = new QSpinBox; m_endBox->setRange(2,1000000);      m_endBox->setValue(100);
    m_runnersBox  = new QSpinBox; m_runnersBox->setRange(1,100);      m_runnersBox->setValue(3);
    m_startMaxBox = new QSpinBox; m_startMaxBox->setRange(1,1000000); m_startMaxBox->setValue(3);
    cfgLayout->addRow("Start:",     m_startBox);
    cfgLayout->addRow("End:",       m_endBox);
    cfgLayout->addRow("Runners:",   m_runnersBox);
    cfgLayout->addRow("Start max:", m_startMaxBox);
    layout->addWidget(cfgGroup);

    auto* optRow = new QHBoxLayout;
    m_preTestBox = new QCheckBox("Pre-test filter");
    m_geoRadio   = new QRadioButton("Geometric"); m_geoRadio->setChecked(true);
    m_numRadio   = new QRadioButton("Numerical");
    optRow->addWidget(m_preTestBox);
    optRow->addWidget(m_geoRadio);
    optRow->addWidget(m_numRadio);
    optRow->addStretch();
    layout->addLayout(optRow);

    m_progress    = new QProgressBar; m_progress->setRange(0, 100);
    m_statusLabel = new QLabel("Not running.");
    layout->addWidget(m_progress);
    layout->addWidget(m_statusLabel);

    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap(true);
    layout->addWidget(m_resultLabel, 1);

    auto* btnRow  = new QHBoxLayout;
    m_runBtn      = new QPushButton("Run");
    m_stopBtn     = new QPushButton("Stop");       m_stopBtn->setEnabled(false);
    m_animateBtn  = new QPushButton("Animate this"); m_animateBtn->setEnabled(false);
    auto* saveBtn = new QPushButton("Save JSON\xe2\x80\xa6");
    btnRow->addWidget(m_runBtn, 2);
    btnRow->addWidget(m_stopBtn, 1);
    btnRow->addWidget(m_animateBtn, 1);
    btnRow->addWidget(saveBtn, 1);
    layout->addLayout(btnRow);

    connect(m_runBtn,  &QPushButton::clicked, this, &RangeTestPanel::onRun);
    connect(m_stopBtn, &QPushButton::clicked, this, &RangeTestPanel::onStop);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { refreshStyle(); });
    refreshStyle();

    connect(m_animateBtn, &QPushButton::clicked, this, [this]() {
        auto* w = new AnimationWidget(m_lastSpeeds, m_lastNumerator, m_lastDenominator);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_lastSpeeds.empty()) return;
        QString path = QFileDialog::getSaveFileName(
            this, "Save result", "", "JSON (*.json)");
        if (path.isEmpty()) return;
        RangeResult rr; rr.status = RangeResult::Status::Clean; rr.speeds = m_lastSpeeds;
        save_result(path.toStdString(), rr, m_lastSpeeds, "range",
                    m_lastNumerator, m_lastDenominator);
    });
}

void RangeTestPanel::onRun() {
    RangeConfig cfg;
    cfg.start_value     = m_startBox->value();
    cfg.end_value       = m_endBox->value();
    cfg.num_runners     = m_runnersBox->value();
    cfg.start_max_value = m_startMaxBox->value();
    cfg.pre_test        = m_preTestBox->isChecked();
    cfg.algorithm = m_geoRadio->isChecked() ? Algorithm::Geometric : Algorithm::Numerical;

    if (cfg.end_value <= cfg.start_value) {
        m_statusLabel->setText("End must be greater than start."); return;
    }
    if (cfg.num_runners >= cfg.end_value - cfg.start_value) {
        m_statusLabel->setText("Runners must be less than range size."); return;
    }

    m_worker = std::make_unique<RangeWorker>(cfg);
    connect(m_worker.get(), &RangeWorker::progress,  this, &RangeTestPanel::onProgress);
    connect(m_worker.get(), &RangeWorker::finished,  this, &RangeTestPanel::onFinished);
    setRunning(true);
    m_worker->start();
}

void RangeTestPanel::onStop() {
    if (m_worker) m_worker->cancel();
}

void RangeTestPanel::onProgress(int current, int total, QString status) {
    m_progress->setMaximum(total);
    m_progress->setValue(current);
    m_statusLabel->setText(status);
}

void RangeTestPanel::onFinished(RangeResult result) {
    setRunning(false);
    if (result.status == RangeResult::Status::Clean) {
        m_hasResult = true; m_resultOk = true;
        m_resultLabel->setText("\xe2\x9c\x93 No violations found.");
    } else if (result.status == RangeResult::Status::ViolationFound) {
        m_hasResult = true; m_resultOk = false;
        m_lastSpeeds = result.speeds;
        auto nr = numerical_method(m_lastSpeeds);
        m_lastNumerator   = nr ? nr->a             : 0;
        m_lastDenominator = nr ? (nr->k1 + nr->k2) : 0;
        m_animateBtn->setEnabled(true);

        QString s;
        for (int i = 0; i < (int)result.speeds.size(); ++i) {
            if (i) s += ", ";
            s += QString::number(result.speeds[i]);
        }
        m_resultLabel->setText("\xe2\x9a\xa0 Possible violation: [" + s + "]");
        emit resultReady(result.speeds, true, m_lastNumerator, m_lastDenominator);
    } else {
        m_hasResult = false; m_resultOk = false;
        m_resultLabel->setText("Cancelled.");
    }
    refreshStyle();
    m_progress->setValue(m_progress->maximum());
}

void RangeTestPanel::setRunning(bool running) {
    m_runBtn->setEnabled(!running);
    m_stopBtn->setEnabled(running);
    if (!running) m_statusLabel->setText("Done.");
}

void RangeTestPanel::refreshStyle() {
    const auto& c  = ThemeManager::instance().colors();
    const QColor fg = m_hasResult ? (m_resultOk ? c.ok : c.fail) : c.muted;
    m_resultLabel->setStyleSheet(
        QString("QLabel{background:%1;color:%2;padding:8px;border-radius:4px;}")
        .arg(c.base.name(), fg.name()));
}
