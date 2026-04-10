#include "thememanager.h"
#include <QApplication>
#include <QGuiApplication>
#include <QProcess>
#include <QWindow>

ThemeManager& ThemeManager::instance() {
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager() {
    // Snapshot the native palette before we touch anything
    m_systemPalette = qApp->palette();
    bool dark = m_systemPalette.color(QPalette::Window).lightness() < 128;
    if (dark) computeDark(); else computeLight();
}

void ThemeManager::apply(ThemeMode mode) {
    m_mode = mode;
    switch (mode) {
        case ThemeMode::Dark:
            applyDarkPalette();
            computeDark();
            break;
        case ThemeMode::Light:
            applyLightPalette();
            computeLight();
            break;
        case ThemeMode::System:
            // Restore the saved native palette
            qApp->setPalette(m_systemPalette);
            {
                bool dark = m_systemPalette.color(QPalette::Window).lightness() < 128;
                if (dark) computeDark(); else computeLight();
            }
            break;
    }
    emit themeChanged();

    // Push the title-bar hint to all windows that are already visible
    for (QWindow* w : QGuiApplication::topLevelWindows()) {
        if (w->isVisible() && w->type() == Qt::Window)
            applyTitleBarHint(w);
    }
}

// ---------------------------------------------------------------------------
// Title bar hint (X11 only)
// ---------------------------------------------------------------------------

bool ThemeManager::isDarkMode() const {
    return (m_mode == ThemeMode::Dark) ||
           (m_mode == ThemeMode::System &&
            m_systemPalette.color(QPalette::Window).lightness() < 128);
}

void ThemeManager::applyTitleBarHint(QWindow* window) {
    if (!window) return;
#ifdef Q_OS_LINUX
    // Only meaningful on X11; silently skipped on Wayland-native
    if (QGuiApplication::platformName() != "xcb") return;
    const QString variant = isDarkMode() ? "dark" : "light";
    QProcess::startDetached("xprop", {
        "-id", QString::number(static_cast<unsigned long>(window->winId())),
        "-format", "_GTK_THEME_VARIANT", "8u",
        "-set",    "_GTK_THEME_VARIANT", variant
    });
#else
    Q_UNUSED(window)
#endif
}

// ---------------------------------------------------------------------------
// Palette helpers
// ---------------------------------------------------------------------------

void ThemeManager::applyDarkPalette() {
    QPalette p;
    p.setColor(QPalette::Window,          QColor("#1e1e2e"));
    p.setColor(QPalette::WindowText,      QColor("#cdd6f4"));
    p.setColor(QPalette::Base,            QColor("#181825"));
    p.setColor(QPalette::AlternateBase,   QColor("#313244"));
    p.setColor(QPalette::Text,            QColor("#cdd6f4"));
    p.setColor(QPalette::BrightText,      QColor("#cdd6f4"));
    p.setColor(QPalette::Button,          QColor("#313244"));
    p.setColor(QPalette::ButtonText,      QColor("#cdd6f4"));
    p.setColor(QPalette::Highlight,       QColor("#89b4fa"));
    p.setColor(QPalette::HighlightedText, QColor("#1e1e2e"));
    p.setColor(QPalette::Link,            QColor("#89b4fa"));
    p.setColor(QPalette::ToolTipBase,     QColor("#313244"));
    p.setColor(QPalette::ToolTipText,     QColor("#cdd6f4"));
    // Disabled state — visibly dimmed
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#6c7086"));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor("#6c7086"));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#6c7086"));
    p.setColor(QPalette::Disabled, QPalette::Highlight,  QColor("#45475a"));
    qApp->setPalette(p);
}

void ThemeManager::applyLightPalette() {
    QPalette p;
    p.setColor(QPalette::Window,          QColor("#eff1f5"));
    p.setColor(QPalette::WindowText,      QColor("#4c4f69"));
    p.setColor(QPalette::Base,            QColor("#dce0e8"));
    p.setColor(QPalette::AlternateBase,   QColor("#e6e9ef"));
    p.setColor(QPalette::Text,            QColor("#4c4f69"));
    p.setColor(QPalette::BrightText,      QColor("#4c4f69"));
    p.setColor(QPalette::Button,          QColor("#ccd0da"));
    p.setColor(QPalette::ButtonText,      QColor("#4c4f69"));
    p.setColor(QPalette::Highlight,       QColor("#1e66f5"));
    p.setColor(QPalette::HighlightedText, QColor("#eff1f5"));
    p.setColor(QPalette::Link,            QColor("#1e66f5"));
    p.setColor(QPalette::ToolTipBase,     QColor("#dce0e8"));
    p.setColor(QPalette::ToolTipText,     QColor("#4c4f69"));
    // Disabled state
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#9ca0b0"));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor("#9ca0b0"));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#9ca0b0"));
    p.setColor(QPalette::Disabled, QPalette::Highlight,  QColor("#acb0be"));
    qApp->setPalette(p);
}

// ---------------------------------------------------------------------------
// Token tables — Catppuccin Mocha (dark) / Catppuccin Latte (light)
// ---------------------------------------------------------------------------

void ThemeManager::computeDark() {
    m_colors.base      = QColor("#181825");
    m_colors.ok        = QColor("#a6e3a1");
    m_colors.fail      = QColor("#f38ba8");
    m_colors.muted     = QColor("#cdd6f4");
    m_colors.highlight = QColor("#89b4fa");
    m_colors.canvasBg  = QColor("#1e1e2e");
    m_colors.track     = QColor("#313244");
    m_colors.originDot = QColor("#f38ba8");
    m_colors.zoneAlpha = 0.18;
    // Mocha pastel palette — designed for dark backgrounds
    m_colors.runners = {
        QColor("#89b4fa"), QColor("#cba6f7"), QColor("#fab387"), QColor("#f9e2af"),
        QColor("#a6e3a1"), QColor("#94e2d5"), QColor("#89dceb"), QColor("#f38ba8")
    };
}

void ThemeManager::computeLight() {
    m_colors.base      = QColor("#e6e9ef");
    m_colors.ok        = QColor("#40a02b");
    m_colors.fail      = QColor("#d20f39");
    m_colors.muted     = QColor("#4c4f69");
    m_colors.highlight = QColor("#1e66f5");
    m_colors.canvasBg  = QColor("#eff1f5");
    m_colors.track     = QColor("#acb0be");
    m_colors.originDot = QColor("#d20f39");
    m_colors.zoneAlpha = 0.35;
    // Latte saturated palette — designed for light backgrounds
    m_colors.runners = {
        QColor("#1e66f5"), QColor("#8839ef"), QColor("#e04d00"), QColor("#df8e1d"),
        QColor("#40a02b"), QColor("#179299"), QColor("#04a5e5"), QColor("#d20f39")
    };
}
