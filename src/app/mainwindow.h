#pragma once
#include <QMainWindow>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <vector>

class ManualTestPanel;
class RangeTestPanel;
class VerifyFilePanel;
class OptionsPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void addToHistory(const std::vector<int>& speeds, bool violation);

protected:
    void showEvent(QShowEvent* event) override;

private:
    QButtonGroup*    m_navGroup{};
    QPushButton*     m_navManual{};
    QPushButton*     m_navRange{};
    QPushButton*     m_navVerify{};
    QPushButton*     m_navOptions{};
    QStackedWidget*  m_stack{};
    QListWidget*     m_history{};
    ManualTestPanel* m_manualPanel{};
    RangeTestPanel*  m_rangePanel{};
    VerifyFilePanel* m_verifyPanel{};
    OptionsPanel*    m_optionsPanel{};

    void buildLayout();
};
