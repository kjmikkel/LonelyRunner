#pragma once
#include "data_structures.h"
#include <atomic>
#include <functional>

// Sequential implementation used by tests and as the per-task unit
// for the parallel version in rangeworker.cpp.
RangeResult range_test_sequential(
    const RangeConfig& config,
    std::atomic<bool>& cancel_flag,
    std::function<void(int current, int total)> progress_cb);
