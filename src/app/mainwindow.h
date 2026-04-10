#pragma once
#include <QMainWindow>
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

private slots:
    void onNavSelected(int row);

private:
    QListWidget*     m_nav{};
    QStackedWidget*  m_stack{};
    QListWidget*     m_history{};
    ManualTestPanel* m_manualPanel{};
    RangeTestPanel*  m_rangePanel{};
    VerifyFilePanel* m_verifyPanel{};
    OptionsPanel*    m_optionsPanel{};

    void buildLayout();
};
