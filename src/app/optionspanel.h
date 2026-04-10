#pragma once
#include <QWidget>
#include <QRadioButton>
#include "thememanager.h"

class OptionsPanel : public QWidget {
    Q_OBJECT
public:
    explicit OptionsPanel(QWidget* parent = nullptr);

private:
    QRadioButton* m_systemRadio{};
    QRadioButton* m_lightRadio{};
    QRadioButton* m_darkRadio{};

    void select(ThemeMode mode);
};
