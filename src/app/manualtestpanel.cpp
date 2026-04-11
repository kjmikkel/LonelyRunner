#include "manualtestpanel.h"
#include "animationwidget.h"
#include "geometric.h"
#include "numerical.h"
#include "prime_modular.h"
#include "util.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

ManualTestPanel::ManualTestPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* inputGroup  = new QGroupBox("Runner Speeds");
    auto* inputLayout = new QHBoxLayout(inputGroup);
    m_speedsInput = new QLineEdit("3, 7, 11, 13");
    auto* importBtn = new QPushButton("Import JSON");
    inputLayout->addWidget(m_speedsInput);
    inputLayout->addWidget(importBtn);
    layout->addWidget(inputGroup);

    auto* optBox    = new QGroupBox("Options");
    auto* optLayout = new QHBoxLayout(optBox);
    m_preTestBox = new QCheckBox("Pre-test filter");
    m_findMaxBox = new QCheckBox("Find maximum t");
    optLayout->addWidget(m_preTestBox);
    optLayout->addWidget(m_findMaxBox);
    optLayout->addStretch();
    layout->addWidget(optBox);

    auto* algoBox    = new QGroupBox("Algorithm");
    auto* algoLayout = new QHBoxLayout(algoBox);
    m_geoRadio   = new QRadioButton("Geometric");
    m_numRadio   = new QRadioButton("Numerical");
    m_primeRadio = new QRadioButton("Prime Modular");
    m_geoRadio->setChecked(true);
    algoLayout->addWidget(m_geoRadio);
    algoLayout->addWidget(m_numRadio);
    algoLayout->addWidget(m_primeRadio);
    algoLayout->addStretch();
    layout->addWidget(algoBox);

    m_resultLabel = new QLabel("No result yet.");
    m_resultLabel->setWordWrap(true);
    layout->addWidget(m_resultLabel, 1);

    auto* btnRow  = new QHBoxLayout;
    auto* runBtn  = new QPushButton("Run");
    runBtn->setDefault(true);
    m_animateBtn  = new QPushButton("Animate this");
    m_animateBtn->setEnabled(false);
    btnRow->addWidget(runBtn, 2);
    btnRow->addWidget(m_animateBtn, 1);
    layout->addLayout(btnRow);

    connect(runBtn,       &QPushButton::clicked, this, &ManualTestPanel::onRun);
    connect(importBtn,    &QPushButton::clicked, this, &ManualTestPanel::onImport);
    connect(m_animateBtn, &QPushButton::clicked, this, &ManualTestPanel::onAnimate);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { refreshStyle(); });
    refreshStyle();
}

void ManualTestPanel::setSpeedText(const QString& text) {
    m_speedsInput->setText(text);
}

std::vector<int> ManualTestPanel::parseSpeeds() const {
    std::vector<int> v;
    for (const QString& part : m_speedsInput->text().split(',')) {
        bool ok; int n = part.trimmed().toInt(&ok);
        if (ok && n > 0) v.push_back(n);
    }
    return v;
}

void ManualTestPanel::onRun() {
    m_lastSpeeds = parseSpeeds();
    if (m_lastSpeeds.empty()) {
        showResult("Enter at least one positive integer speed.", false);
        return;
    }

    if (m_preTestBox->isChecked() && check_for_solution(m_lastSpeeds)) {
        showResult("Pre-test: trivial solution detected.", true);
        m_animateBtn->setEnabled(false);
        return;
    }

    bool find_max = m_findMaxBox->isChecked();
    QString algo_name;
    bool valid = false;
    m_lastNumerator = m_lastDenominator = 0;

    if (m_geoRadio->isChecked()) {
        algo_name = "Geometric";
        auto r = geometric_method(m_lastSpeeds);
        if (r && r->found) {
            valid = is_valid(*r, m_lastSpeeds);
            const auto& p   = r->point;
            m_lastNumerator   = p.local_position + p.rounds * (p.number_of_runners + 1);
            m_lastDenominator = p.speed * (p.number_of_runners + 1);
        }
    } else if (m_numRadio->isChecked()) {
        algo_name = "Numerical";
        auto r = numerical_method(m_lastSpeeds, find_max);
        if (r && r->found) {
            valid = is_valid(*r, m_lastSpeeds);
            m_lastNumerator   = r->a;
            m_lastDenominator = r->k1 + r->k2;
        }
    } else {
        // Prime Modular: t = a / ((n+1)*p) for small primes p.
        // Based on Rosenfeld (arXiv:2509.14111) and Trakulthongchai (arXiv:2511.22427).
        algo_name = "Prime Modular";
        auto r = prime_modular_method(m_lastSpeeds);
        if (r && r->found) {
            valid = is_valid(*r, m_lastSpeeds);
            m_lastNumerator   = r->a;
            m_lastDenominator = (static_cast<int>(m_lastSpeeds.size()) + 1) * r->prime;
        }
    }

    if (valid) {
        showResult(QString("%1: Solution found \xe2\x9c\x93  t = %2 / %3")
                   .arg(algo_name).arg(m_lastNumerator).arg(m_lastDenominator), true);
        m_animateBtn->setEnabled(true);
        emit resultReady(m_lastSpeeds, true, m_lastNumerator, m_lastDenominator);
    } else {
        showResult(QString("%1: No valid solution found.").arg(algo_name), false);
        m_animateBtn->setEnabled(false);
        emit resultReady(m_lastSpeeds, false, 0, 0);
    }
}

void ManualTestPanel::onImport() {
    QString path = QFileDialog::getOpenFileName(this, "Open JSON", "", "JSON (*.json)");
    if (path.isEmpty()) return;
    try {
        auto speeds = read_speeds_from_json(path.toStdString());
        QString text;
        for (int i = 0; i < (int)speeds.size(); ++i) {
            if (i > 0) text += ", ";
            text += QString::number(speeds[i]);
        }
        m_speedsInput->setText(text);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Import failed", e.what());
    }
}

void ManualTestPanel::onAnimate() {
    auto* w = new AnimationWidget(m_lastSpeeds, m_lastNumerator, m_lastDenominator);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void ManualTestPanel::showResult(const QString& text, bool ok) {
    m_hasResult = true;
    m_resultOk  = ok;
    m_resultLabel->setText(text);
    refreshStyle();
}

void ManualTestPanel::refreshStyle() {
    const auto& c  = ThemeManager::instance().colors();
    const QColor fg = m_hasResult ? (m_resultOk ? c.ok : c.fail) : c.muted;
    m_resultLabel->setStyleSheet(
        QString("QLabel{background:%1;color:%2;padding:8px;border-radius:4px;}")
        .arg(c.base.name(), fg.name()));
}
