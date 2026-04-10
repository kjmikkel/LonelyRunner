#include "numerical.h"
#include <NTL/ZZ.h>
using namespace NTL;

// Fix #5: break is now inside the if(!valid) block
// Fix #6: uses NTL % operator instead of O(n) subtraction loop
static bool is_valid_internal(ZZ P, ZZ Q, std::span<const int> speeds) {
    long n = static_cast<long>(speeds.size());
    for (int speed : speeds) {
        ZZ dist_from_start = (to_ZZ(speed) * P) % Q;
        ZZ dist_to_end     = Q - dist_from_start;
        ZZ compare = (dist_from_start < dist_to_end) ? dist_from_start : dist_to_end;
        if (compare * (n + 1) < Q)
            return false;
    }
    return true;
}

bool is_valid(const NumResult& result, std::span<const int> speeds) {
    ZZ P = to_ZZ(result.a);
    ZZ Q = to_ZZ(result.k1) + to_ZZ(result.k2);
    return is_valid_internal(P, Q, speeds);
}

bool is_valid(const GeoResult& result, std::span<const int> speeds) {
    const EventPoint& p = result.point;
    ZZ P = to_ZZ(p.local_position) + to_ZZ(p.rounds) * (p.number_of_runners + 1);
    ZZ Q = to_ZZ(p.speed) * (p.number_of_runners + 1);
    return is_valid_internal(P, Q, speeds);
}

bool check_for_solution(std::span<const int> speeds) {
    int n = static_cast<int>(speeds.size());
    for (int number = 2; number < n + 2; ++number) {
        bool divides_any = false;
        for (int speed : speeds) {
            if (speed % number == 0) { divides_any = true; break; }
        }
        if (!divides_any) return true;
    }
    return false;
}

std::optional<NumResult> numerical_method(std::span<const int> speeds, bool find_maximum) {
    const int length = static_cast<int>(speeds.size());
    if (length == 0) return std::nullopt;

    if (length == 1) {
        NumResult r;
        r.found = true;
        r.k1 = speeds[0] * (length + 1);
        r.k2 = 0;
        r.a  = 1;
        return r;
    }

    bool no_candidate = true;
    int  candidate_k1 = 0, candidate_k2 = 0, candidate_a = 0;
    ZZ   candidate_k;   // Fix #7: no longer initialised to -1; guarded by no_candidate
                        // Fix #8: updated whenever a better candidate is stored

    for (int fi = 0; fi < length - 1; ++fi) {
        for (int si = fi + 1; si < length; ++si) {
            int k_int = speeds[fi] + speeds[si];
            ZZ  k     = to_ZZ(k_int);

            for (int a = 1; a < k_int; ++a) {
                ZZ zz_a = to_ZZ(a);
                bool test_valid = true;

                for (int speed : speeds) {
                    ZZ dist_from_start = (to_ZZ(speed) * zz_a) % k;
                    ZZ dist_to_end     = k - dist_from_start;
                    ZZ compare = (dist_from_start < dist_to_end)
                                 ? dist_from_start : dist_to_end;
                    if (compare * (length + 1) < k) {
                        test_valid = false;
                        break;
                    }
                }

                if (test_valid) {
                    // Fix #8: compare k/a vs candidate_k/candidate_a (larger t wins for max mode)
                    bool is_better = no_candidate ||
                                     (k * candidate_a < candidate_k * a);
                    if (is_better) {
                        no_candidate  = false;
                        candidate_k1  = speeds[fi];
                        candidate_k2  = speeds[si];
                        candidate_a   = a;
                        candidate_k   = k;    // Fix #8: was missing in original
                    }
                    if (!find_maximum)
                        return NumResult{true, candidate_k1, candidate_k2, candidate_a};
                }
            }
        }
    }

    if (no_candidate) return std::nullopt;
    return NumResult{true, candidate_k1, candidate_k2, candidate_a};
}
