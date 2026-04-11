#include "numerical.h"
#include <NTL/ZZ.h>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Check whether time t = P/Q is a valid lonely time for all runners.
// A runner with speed v is lonely at t iff its distance from the start point
// (min of position and 1-position on the unit circle) is >= 1/(n+1).
// In integer arithmetic: min(v*P mod Q, Q - v*P mod Q) * (n+1) >= Q.
static bool is_valid_internal(NTL::ZZ P, NTL::ZZ Q, std::span<const int> speeds) {
    NTL::ZZ n_plus_1 = NTL::to_ZZ(static_cast<long>(speeds.size()) + 1);
    for (int speed : speeds) {
        NTL::ZZ dist_from_start = (NTL::to_ZZ(speed) * P) % Q;
        NTL::ZZ dist_to_end     = Q - dist_from_start;
        NTL::ZZ dist            = (dist_from_start < dist_to_end)
                                  ? dist_from_start : dist_to_end;
        if (dist * n_plus_1 < Q)
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool is_valid(const NumResult& result, std::span<const int> speeds) {
    NTL::ZZ P = NTL::to_ZZ(result.a);
    NTL::ZZ Q = NTL::to_ZZ(result.k1) + NTL::to_ZZ(result.k2);
    return is_valid_internal(P, Q, speeds);
}

bool is_valid(const GeoResult& result, std::span<const int> speeds) {
    const EventPoint& p = result.point;
    NTL::ZZ P = NTL::to_ZZ(p.local_position)
              + NTL::to_ZZ(p.rounds) * (p.number_of_runners + 1);
    NTL::ZZ Q = NTL::to_ZZ(p.speed) * (p.number_of_runners + 1);
    return is_valid_internal(P, Q, speeds);
}

// Quick pre-filter: if any k in {2,..,n+1} fails to divide every speed,
// then t = 1/k is a valid lonely time (each runner's position k*speed mod k ≠ 0,
// giving distance >= 1/k >= 1/(n+1)).
bool check_for_solution(std::span<const int> speeds) {
    const int n = static_cast<int>(speeds.size());
    for (int k = 2; k <= n + 1; ++k) {
        bool divides_any = false;
        for (int speed : speeds) {
            if (speed % k == 0) { divides_any = true; break; }
        }
        if (!divides_any) return true;
    }
    return false;
}

// Search for a lonely time of the form t = a/k where k = speeds[i] + speeds[j]
// for some pair (i, j).  This family of denominators covers the critical times
// where two runners are symmetrically placed at the zone boundary — it does not
// exhaustively cover all possible lonely times, but works well for small n.
//
// Speed 0 is not a valid input (a stationary runner is never lonely); the method
// will return nullopt for any input containing 0.
//
// If find_maximum is true, the search continues after the first valid time to
// find the latest one (useful for displaying the "hardest" configuration).
std::optional<NumResult> numerical_method(std::span<const int> speeds, bool find_maximum) {
    const int n = static_cast<int>(speeds.size());
    if (n == 0) return std::nullopt;

    // Single-runner special case: lonely at t = 1/(2*speed)
    if (n == 1) {
        return NumResult{true, speeds[0] * (n + 1), 0, 1};
    }

    // Precompute ZZ representations of each speed to avoid repeated conversion
    // inside the triple loop (each to_ZZ() allocates; converting n speeds once
    // saves O(n^2 * max_k) allocations).
    std::vector<NTL::ZZ> zz_speeds(n);
    for (int i = 0; i < n; ++i)
        zz_speeds[i] = NTL::to_ZZ(speeds[i]);

    bool      no_candidate = true;
    int       best_k1 = 0, best_k2 = 0, best_a = 0;
    NTL::ZZ   best_k;   // best_a / best_k is the largest valid time found so far

    for (int first_idx = 0; first_idx < n - 1; ++first_idx) {
        for (int second_idx = first_idx + 1; second_idx < n; ++second_idx) {
            // Use ZZ arithmetic for k to avoid int overflow for large speeds
            NTL::ZZ k     = zz_speeds[first_idx] + zz_speeds[second_idx];
            long    k_long = NTL::conv<long>(k);

            for (long a = 1; a < k_long; ++a) {
                NTL::ZZ zz_a = NTL::to_ZZ(a);
                bool valid = true;

                for (int i = 0; i < n; ++i) {
                    NTL::ZZ dist_from_start = (zz_speeds[i] * zz_a) % k;
                    NTL::ZZ dist_to_end     = k - dist_from_start;
                    NTL::ZZ dist = (dist_from_start < dist_to_end)
                                   ? dist_from_start : dist_to_end;
                    if (dist * (n + 1) < k) { valid = false; break; }
                }

                if (valid) {
                    // Compare a/k with best_a/best_k; keep the larger one
                    bool is_better = no_candidate
                                  || (k * best_a < best_k * a);
                    if (is_better) {
                        no_candidate = false;
                        best_k1 = speeds[first_idx];
                        best_k2 = speeds[second_idx];
                        best_a  = static_cast<int>(a);
                        best_k  = k;
                    }
                    if (!find_maximum)
                        return NumResult{true, best_k1, best_k2, best_a};
                }
            }
        }
    }

    if (no_candidate) return std::nullopt;
    return NumResult{true, best_k1, best_k2, best_a};
}
