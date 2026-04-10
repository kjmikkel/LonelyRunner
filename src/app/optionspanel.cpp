#include "optionspanel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>

OptionsPanel::OptionsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    layout->addWidget(new QLabel("<h3>Options</h3>"));

    auto* group = new QGroupBox("Color theme");
    auto* vbox  = new QVBoxLayout(group);
    vbox->setSpacing(8);
    m_systemRadio = new QRadioButton("System  (follow the OS preference)");
    m_lightRadio  = new QRadioButton("Light");
    m_darkRadio   = new QRadioButton("Dark");
    vbox->addWidget(m_systemRadio);
    vbox->addWidget(m_lightRadio);
    vbox->addWidget(m_darkRadio);
    layout->addWidget(group);
    layout->addStretch();

    // Reflect the currently active mode
    switch (ThemeManager::instance().currentMode()) {
        case ThemeMode::Light:  m_lightRadio->setChecked(true);  break;
        case ThemeMode::Dark:   m_darkRadio->setChecked(true);   break;
        default:                m_systemRadio->setChecked(true); break;
    }

    connect(m_systemRadio, &QRadioButton::clicked, this,
            [this]() { select(ThemeMode::System); });
    connect(m_lightRadio, &QRadioButton::clicked, this,
            [this]() { select(ThemeMode::Light); });
    connect(m_darkRadio, &QRadioButton::clicked, this,
            [this]() { select(ThemeMode::Dark); });
}

void OptionsPanel::select(ThemeMode mode) {
    QSettings s("LonelyRunnerVerifier", "LonelyRunnerVerifier");
    switch (mode) {
        case ThemeMode::Light:  s.setValue("theme/mode", "Light");  break;
        case ThemeMode::Dark:   s.setValue("theme/mode", "Dark");   break;
        default:                s.setValue("theme/mode", "System"); break;
    }
    ThemeManager::instance().apply(mode);
}
