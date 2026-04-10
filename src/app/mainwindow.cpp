#include "mainwindow.h"
#include "manualtestpanel.h"
#include "rangetestpanel.h"
#include "verifyfilepanel.h"
#include "optionspanel.h"
#include "thememanager.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
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

void MainWindow::buildLayout() {
    // ---- Sidebar nav ----
    m_nav = new QListWidget;
    m_nav->addItem("Manual Test");
    m_nav->addItem("Range Test");
    m_nav->addItem("Animation");   // row 2 — special: opens a floating window
    m_nav->addItem("Verify File");
    m_nav->addItem("Options");
    m_nav->setFixedWidth(145);
    m_nav->setCurrentRow(0);

    // ---- History list ----
    m_history = new QListWidget;
    m_history->setFixedWidth(145);

    auto* sidebar    = new QWidget;
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(4, 4, 4, 4);
    sideLayout->addWidget(m_nav, 3);
    sideLayout->addWidget(new QLabel("History"), 0);
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
    connect(m_nav, &QListWidget::currentRowChanged,
            this,  &MainWindow::onNavSelected);

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
        m_nav->setCurrentRow(0);
        m_manualPanel->setSpeedText(label);
    });
}

void MainWindow::onNavSelected(int row) {
    if (row == 2) {
        // Animation has no embedded panel — it opens from "Animate this" buttons
        QMessageBox::information(this, "Animation",
            "Run a Manual Test or Range Test first,\n"
            "then click \"Animate this\" to open the animation window.");
        m_nav->setCurrentRow(m_stack->currentIndex());
        return;
    }
    // Nav rows 0,1 map directly; rows 3,4 skip the animation slot → subtract 1
    int stackIndex = (row < 2) ? row : row - 1;
    m_stack->setCurrentIndex(stackIndex);
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
