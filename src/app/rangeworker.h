#pragma once
#include <QObject>
#include <QFutureWatcher>
#include <atomic>
#include "range_test.h"

class RangeWorker : public QObject {
    Q_OBJECT
public:
    explicit RangeWorker(RangeConfig config, QObject* parent = nullptr);
    void start();
    void cancel();

signals:
    void progress(int current, int total, QString status);
    void finished(RangeResult result);

private:
    RangeConfig m_config;
    std::atomic<bool> m_cancel{false};
    QFutureWatcher<RangeResult> m_watcher;
};
