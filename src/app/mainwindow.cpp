#include "mainwindow.h"
#include "manualtestpanel.h"
#include "rangetestpanel.h"
#include "verifyfilepanel.h"
#include "optionspanel.h"
#include "thememanager.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QShowEvent>
#include <algorithm>

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    ThemeManager::instance().applyTitleBarHint(windowHandle());
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Lonely Runner Verifier");
    resize(920, 620);
    buildLayout();
}

static QPushButton* makeNavBtn(const QString& icon, const QString& label) {
    auto* b = new QPushButton(icon + "  " + label);
    b->setCheckable(true);
    b->setFlat(true);
    b->setProperty("navRole", true);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setMinimumHeight(40);
    return b;
}

void MainWindow::buildLayout() {
    // ---- Nav buttons ----
    m_navManual  = makeNavBtn("\xe2\x97\x86", "Manual Test");   // ◆
    m_navRange   = makeNavBtn("\xe2\x89\x8b", "Range Test");    // ≋
    m_navVerify  = makeNavBtn("\xe2\x9c\x94", "Verify File");   // ✔
    m_navOptions = makeNavBtn("\xe2\x9a\x99", "Options");       // ⚙

    m_navGroup = new QButtonGroup(this);
    m_navGroup->addButton(m_navManual,  0);
    m_navGroup->addButton(m_navRange,   1);
    m_navGroup->addButton(m_navVerify,  2);
    m_navGroup->addButton(m_navOptions, 3);
    m_navManual->setChecked(true);

    auto* navContainer = new QWidget;
    auto* navLayout    = new QVBoxLayout(navContainer);
    navLayout->setContentsMargins(4, 4, 4, 4);
    navLayout->setSpacing(2);
    navLayout->addWidget(m_navManual);
    navLayout->addWidget(m_navRange);
    navLayout->addWidget(m_navVerify);
    navLayout->addWidget(m_navOptions);
    navLayout->addStretch();

    // ---- History list ----
    m_history = new QListWidget;
    m_history->setFixedWidth(145);

    auto* histLabel = new QLabel("HISTORY");
    histLabel->setStyleSheet(
        "QLabel { font-size: 10px; font-weight: bold; color: palette(mid); "
        "padding: 4px 8px 2px 8px; letter-spacing: 1px; }");

    auto* sidebar    = new QWidget;
    sidebar->setFixedWidth(160);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(4, 4, 4, 4);
    sideLayout->setSpacing(0);
    sideLayout->addWidget(navContainer, 3);
    sideLayout->addWidget(histLabel, 0);
    sideLayout->addWidget(m_history, 1);

    // ---- Panels ----
    m_manualPanel  = new ManualTestPanel(this);
    m_rangePanel   = new RangeTestPanel(this);
    m_verifyPanel  = new VerifyFilePanel(this);
    m_optionsPanel = new OptionsPanel(this);

    m_stack = new QStackedWidget;
    m_stack->addWidget(m_manualPanel);   // 0
    m_stack->addWidget(m_rangePanel);    // 1
    m_stack->addWidget(m_verifyPanel);   // 2
    m_stack->addWidget(m_optionsPanel);  // 3

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(sidebar);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    // ---- Connections ----
    connect(m_navGroup, &QButtonGroup::idClicked,
            m_stack, &QStackedWidget::setCurrentIndex);

    connect(m_manualPanel, &ManualTestPanel::resultReady,
            this, [this](std::vector<int> speeds, bool valid, int, int) {
        addToHistory(speeds, !valid);
    });

    connect(m_rangePanel, &RangeTestPanel::resultReady,
            this, [this](std::vector<int> speeds, bool violation, int, int) {
        addToHistory(speeds, violation);
    });

    connect(m_history, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
        // Strip the "✓ " / "⚠ " prefix and send text back to the manual panel
        QString label = item->text().mid(2);
        m_navGroup->button(0)->setChecked(true);
        m_stack->setCurrentIndex(0);
        m_manualPanel->setSpeedText(label);
    });
}

void MainWindow::addToHistory(const std::vector<int>& speeds, bool violation) {
    QString label = violation ? "\xe2\x9a\xa0 " : "\xe2\x9c\x93 ";
    int show = static_cast<int>(std::min(speeds.size(), size_t(4)));
    for (int i = 0; i < show; ++i) {
        if (i > 0) label += ",";
        label += QString::number(speeds[i]);
    }
    if (speeds.size() > 4) label += "\xe2\x80\xa6";
    m_history->insertItem(0, label);
}
