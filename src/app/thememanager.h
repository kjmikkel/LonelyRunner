#pragma once
#include <QObject>
#include <QColor>
#include <QPalette>
#include <array>

class QWindow;

enum class ThemeMode { System, Light, Dark };

struct ThemeColors {
    QColor base;       // result / display panel background
    QColor ok;         // success text (green)
    QColor fail;       // error text (red)
    QColor muted;      // neutral / info text
    QColor highlight;  // accent color (blue, for time label etc.)
    QColor canvasBg;   // animation canvas background
    QColor track;      // animation track ring
    QColor originDot;  // origin marker on track
    qreal  zoneAlpha{0.18}; // opacity of the lonely-zone sector
    std::array<QColor, 8> runners{};  // per-theme runner dot palette
};

class ThemeManager : public QObject {
    Q_OBJECT
public:
    static ThemeManager& instance();

    // Apply a theme mode (also saves it internally). Emits themeChanged().
    void apply(ThemeMode mode);

    // Set _GTK_THEME_VARIANT on an X11 window so the WM title bar follows the theme.
    // Call after show() or from showEvent(). No-op on non-X11 or if xprop is absent.
    void applyTitleBarHint(QWindow* window);

    ThemeMode         currentMode() const { return m_mode; }
    const ThemeColors& colors()    const { return m_colors; }
    bool              isDarkMode() const;

signals:
    void themeChanged();

private:
    ThemeManager();

    void applyDarkPalette();
    void applyLightPalette();
    void applyGlobalStylesheet();
    void computeDark();
    void computeLight();

    ThemeMode   m_mode{ThemeMode::System};
    ThemeColors m_colors{};
    QPalette    m_systemPalette;  // snapshot of palette at startup
};
