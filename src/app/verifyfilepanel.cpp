#include "verifyfilepanel.h"
#include "util.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

VerifyFilePanel::VerifyFilePanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* row = new QHBoxLayout;
    auto* btn = new QPushButton("Choose result file\xe2\x80\xa6");
    m_pathLabel = new QLabel("No file selected.");
    m_pathLabel->setWordWrap(true);
    row->addWidget(btn);
    row->addWidget(m_pathLabel, 1);
    layout->addLayout(row);

    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap(true);
    layout->addWidget(m_resultLabel, 1);
    layout->addStretch();

    connect(btn, &QPushButton::clicked, this, &VerifyFilePanel::onChooseFile);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { refreshStyle(); });
    refreshStyle();
}

void VerifyFilePanel::onChooseFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open result file", "", "JSON (*.json)");
    if (path.isEmpty()) return;
    m_pathLabel->setText(path);

    try {
        bool ok = verify_result(path.toStdString());
        m_hasResult = true;
        m_resultOk  = ok;
        if (ok) {
            m_resultLabel->setText(
                "\xe2\x9c\x93  PASS \xe2\x80\x94 the saved time t is valid for these speeds.");
        } else {
            m_resultLabel->setText(
                "\xe2\x9c\x97  FAIL \xe2\x80\x94 the saved time t does NOT satisfy the conjecture.\n"
                "This may be a genuine violation or a corrupted file.");
        }
        refreshStyle();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
            QString("Could not read file:\n%1").arg(e.what()));
    }
}

void VerifyFilePanel::refreshStyle() {
    const auto& c  = ThemeManager::instance().colors();
    const QColor fg = m_hasResult ? (m_resultOk ? c.ok : c.fail) : c.muted;
    m_resultLabel->setStyleSheet(
        QString("QLabel{background:%1;color:%2;padding:12px;border-radius:4px;}")
        .arg(c.base.name(), fg.name()));
}
