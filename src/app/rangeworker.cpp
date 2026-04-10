#include "rangeworker.h"
#include <QtConcurrent/QtConcurrent>
#include <QVector>
#include <numeric>

RangeWorker::RangeWorker(RangeConfig config, QObject* parent)
    : QObject(parent), m_config(std::move(config)) {

    connect(&m_watcher, &QFutureWatcher<RangeResult>::finished,
            this, [this]() { emit finished(m_watcher.result()); });
}

void RangeWorker::start() {
    m_cancel.store(false);

    const int range = m_config.end_value - m_config.start_value;
    QVector<int> top_indices(range);
    std::iota(top_indices.begin(), top_indices.end(), 0);

    const int total         = range;
    std::atomic<int> done{0};
    const RangeConfig cfg   = m_config;
    std::atomic<bool>& cref = m_cancel;

    auto future = QtConcurrent::mappedReduced<RangeResult>(
        top_indices,
        // Map: test one top-value index, return partial RangeResult
        [cfg, &cref, &done, total, this](int top_idx) -> RangeResult {
            if (cref.load()) {
                RangeResult r; r.status = RangeResult::Status::Cancelled; return r;
            }
            // Build a config that covers only this single top value
            RangeConfig single      = cfg;
            single.start_max_value  = cfg.start_value + top_idx;
            single.end_value        = cfg.start_value + top_idx + 1;

            std::atomic<bool> local_cancel{false};
            RangeResult r = range_test_sequential(single, local_cancel, [](int,int){});

            int n = ++done;
            emit progress(n, total,
                          QString("Testing top value %1")
                          .arg(cfg.start_value + top_idx));
            if (r.status == RangeResult::Status::ViolationFound)
                cref.store(true);
            return r;
        },
        // Reduce: keep first violation found, accumulate elapsed time
        [](RangeResult& acc, const RangeResult& part) {
            if (part.status == RangeResult::Status::ViolationFound &&
                acc.status  != RangeResult::Status::ViolationFound)
                acc = part;
            else
                acc.elapsed += part.elapsed;
        },
        QtConcurrent::ReduceOption::UnorderedReduce
    );

    m_watcher.setFuture(future);
}

void RangeWorker::cancel() {
    m_cancel.store(true);
    m_watcher.cancel();
}
