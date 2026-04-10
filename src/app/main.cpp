#include <QApplication>
#include <QSettings>
#include "mainwindow.h"
#include "thememanager.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Lonely Runner Verifier");
    app.setApplicationVersion("2.0");

    // Restore saved theme *before* creating any windows so the initial
    // palette is correct and there is no flash of the wrong colours.
    QSettings s("LonelyRunnerVerifier", "LonelyRunnerVerifier");
    const QString saved = s.value("theme/mode", "System").toString();
    ThemeMode mode = ThemeMode::System;
    if (saved == "Light") mode = ThemeMode::Light;
    else if (saved == "Dark") mode = ThemeMode::Dark;
    ThemeManager::instance().apply(mode);

    MainWindow w;
    w.show();
    return app.exec();
}
