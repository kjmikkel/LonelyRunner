#include "range_test.h"
#include "geometric.h"
#include "numerical.h"
#include <vector>
#include <chrono>
#include <algorithm>

// Recursively generates all strictly-increasing k-tuples from number_array
// (indices 0..max_index-1) and tests each one for a conjecture violation.
//
// slot: the current position being filled (0-based, innermost first)
// max_index: only entries 0..max_index-1 are available for this slot
//   (enforces strictly increasing order: each slot picks an index less than
//   the one above it, so the final speeds array is strictly sorted)
//
// Returns true if a violation is found (caller should stop recursing).
static bool recursive_test(std::vector<int>& array,
                            const std::vector<int>& number_array,
                            int slot,
                            int num_runners,
                            int max_index,
                            bool pre_test,
                            std::atomic<bool>& cancel) {
    if (cancel.load(std::memory_order_relaxed)) return false;

    for (int candidate_idx = 0; candidate_idx < max_index; ++candidate_idx) {
        array[slot] = number_array[candidate_idx];

        if (slot + 1 < num_runners - 1) {
            // Not the last inner slot — recurse with a smaller available range
            // so the next slot can only pick indices strictly below candidate_idx
            if (recursive_test(array, number_array,
                                slot + 1, num_runners, candidate_idx,
                                pre_test, cancel))
                return true;
        } else {
            // All slots filled — test this combination
            std::span<const int> speeds(array.data(), num_runners);

            if (pre_test && check_for_solution(speeds))
                continue;   // easy pre-filter: a solution provably exists

            // The geometric method is the authoritative verifier: it sweeps the
            // timeline completely within the FINAL bound.  The numerical method
            // is a "lonely-time finder" whose search space (pair denominators)
            // is not exhaustive, so a nullopt from it does NOT prove a violation.
            // We always use geometric for the violation determination.
            auto result   = geometric_method(speeds);
            bool violation = !result.has_value() || !is_valid(*result, speeds);

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

    // Build the candidate pool: integers [start_value, end_value)
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

    // Find the starting index for the outermost (top) slot.
    // The top slot must be at index >= num_runners-1 (otherwise there aren't
    // enough lower values to fill the inner slots with strictly smaller values).
    // We also skip indices whose values are below start_max_value, which allows
    // resuming a range test from a previous checkpoint.
    int start_top = cfg.num_runners - 1;
    for (int i = 0; i < total; ++i) {
        if (number_array[i] >= cfg.start_max_value) {
            start_top = std::max(start_top, i);
            break;
        }
    }

    std::vector<int> array(cfg.num_runners, 0);

    for (int top_idx = start_top; top_idx < total; ++top_idx) {
        if (cancel_flag.load(std::memory_order_relaxed)) {
            result.status = RangeResult::Status::Cancelled;
            break;
        }

        progress_cb(top_idx - start_top + 1, total - start_top);

        array[cfg.num_runners - 1] = number_array[top_idx];

        // Inner slots pick from indices strictly below top_idx
        bool found = recursive_test(
            array, number_array,
            0, cfg.num_runners, top_idx,
            cfg.pre_test, cancel_flag);

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
