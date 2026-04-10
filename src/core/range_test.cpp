#include "range_test.h"
#include "geometric.h"
#include "numerical.h"
#include <vector>
#include <chrono>
#include <algorithm>

// Recursively fills speed combinations and tests each one.
// Fills slots 0..num_runners-2; the top-value slot (num_runners-1) is pre-set by caller.
// max_index: number of entries in number_array available for this slot (exclusive upper bound).
// Returns true if a violation candidate was found.
static bool recursive_test(std::vector<int>& array,
                            const std::vector<int>& number_array,
                            int slot,          // current slot index (0-based)
                            int num_runners,
                            int max_index,     // entries 0..max_index-1 are available
                            bool pre_test,
                            Algorithm algo,
                            std::atomic<bool>& cancel) {
    if (cancel.load(std::memory_order_relaxed)) return false;

    for (int ci = 0; ci < max_index; ++ci) {
        array[slot] = number_array[ci];

        if (slot + 1 < num_runners - 1) {
            // Not the last inner slot — recurse, restricting to strictly lower index
            if (recursive_test(array, number_array,
                                slot + 1, num_runners, ci,
                                pre_test, algo, cancel))
                return true;
        } else {
            // All slots filled — run the test
            std::span<const int> speeds(array.data(), num_runners);

            if (pre_test && check_for_solution(speeds))
                continue;

            bool violation = false;
            if (algo == Algorithm::Geometric) {
                auto r = geometric_method(speeds);
                violation = !r.has_value() || !is_valid(*r, speeds);
            } else {
                auto r = numerical_method(speeds);
                violation = !r.has_value() || !is_valid(*r, speeds);
            }

            if (violation) return true;
        }
    }
    return false;
}

RangeResult range_test_sequential(
        const RangeConfig& cfg,
        std::atomic<bool>& cancel_flag,
        std::function<void(int, int)> progress_cb) {

    auto t_start = std::chrono::steady_clock::now();

    // Build number_array: integers [start_value, end_value)
    // Fix #12 (off-by-one): end_value is exclusive upper bound
    std::vector<int> number_array;
    number_array.reserve(cfg.end_value - cfg.start_value);
    for (int v = cfg.start_value; v < cfg.end_value; ++v)
        number_array.push_back(v);

    const int total = static_cast<int>(number_array.size());
    RangeResult result;
    result.status = RangeResult::Status::Clean;

    if (total < cfg.num_runners) {
        result.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - t_start);
        return result;
    }

    // Find starting index for the top-value slot
    int start_top = cfg.num_runners - 1;
    for (int i = 0; i < total; ++i) {
        if (number_array[i] >= cfg.start_max_value) {
            start_top = std::max(start_top, i);
            break;
        }
    }

    // Fix #11: use std::vector instead of VLA
    std::vector<int> array(cfg.num_runners, 0);

    for (int top_idx = start_top; top_idx < total; ++top_idx) {
        if (cancel_flag.load(std::memory_order_relaxed)) {
            result.status = RangeResult::Status::Cancelled;
            break;
        }

        progress_cb(top_idx - start_top + 1, total - start_top);

        array[cfg.num_runners - 1] = number_array[top_idx];

        bool found = recursive_test(
            array, number_array,
            0, cfg.num_runners,
            top_idx,   // inner slots pick from indices strictly less than top_idx
            cfg.pre_test, cfg.algorithm, cancel_flag);

        if (found) {
            result.status = RangeResult::Status::ViolationFound;
            result.speeds.assign(array.begin(), array.begin() + cfg.num_runners);
            break;
        }
    }

    result.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - t_start);
    return result;
}
