#include "geometric.h"
#include <queue>
#include <vector>
#include <NTL/ZZ.h>

// ---------------------------------------------------------------------------
// Internal types
// ---------------------------------------------------------------------------

// Each event is augmented with its precomputed time numerator so the
// comparator only needs two multiplications instead of four.
//
// Time model: a runner with speed s enters the lonely zone (distance ≥ 1/(n+1))
// at real time t = (1 + k*(n+1)) / (s*(n+1)) for k = 0, 1, 2, …  and exits at
// t = (n + k*(n+1)) / (s*(n+1)).  Dropping the shared (n+1) denominator, the
// "time numerator" for the k-th START of a runner is (1 + k*(n+1)) and for the
// END it is (n + k*(n+1)).  Comparing two events a and b at times ta/sa and
// tb/sb reduces to cross-multiplying: ta*sb vs tb*sa.
struct QueueEntry {
    EventPoint point;
    NTL::ZZ    time_num;   // precomputed: local_position + rounds*(n+1)
};

// Priority: earliest time pops first.  Tiebreakers (equal time):
//   START before END (preserves full overlap before any exit),
//   lower runner_number first (deterministic),
//   FINAL last (sentinel — only pops if no solution was found in range).
struct CompareEntries {
    bool operator()(const QueueEntry& a, const QueueEntry& b) const {
        // Timing comparison via cross-multiplication (avoids division, handles ZZ)
        NTL::ZZ ta = a.time_num * b.point.speed;
        NTL::ZZ tb = b.time_num * a.point.speed;
        if (ta != tb) return ta > tb;  // later time = lower priority

        // Equal time: enforce total order
        // Type priority: START (0) > END (1) > FINAL (2) — lower value = higher priority
        auto rank = [](PointType t) -> int {
            return t == PointType::START ? 0 : (t == PointType::END ? 1 : 2);
        };
        int ra = rank(a.point.type), rb = rank(b.point.type);
        if (ra != rb) return ra > rb;   // higher rank value = lower priority

        // Same type and same time: lower runner_number wins (arbitrary but stable)
        return a.point.runner_number > b.point.runner_number;
    }
};

using EventQueue = std::priority_queue<QueueEntry,
                                        std::vector<QueueEntry>,
                                        CompareEntries>;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static QueueEntry make_entry(const EventPoint& src, int local_position, PointType type) {
    EventPoint p  = src;
    p.rounds      = src.rounds + 1;
    p.local_position = local_position;
    p.type        = type;
    NTL::ZZ num   = NTL::to_ZZ(p.local_position)
                  + NTL::to_ZZ(p.rounds) * (p.number_of_runners + 1);
    return {p, num};
}

// Push the START and END events for the next zone interval of this runner.
// Called with the previous END event (or the seed) to generate the next pair.
static void push_interval(const EventPoint& prev, EventQueue& q, int n) {
    q.push(make_entry(prev, 1, PointType::START));  // zone entry: position 1/(n+1)
    q.push(make_entry(prev, n, PointType::END));    // zone exit:  position n/(n+1)
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::optional<GeoResult> geometric_method(std::span<const int> speeds) {
    const int n = static_cast<int>(speeds.size());
    if (n < 1) return std::nullopt;

    EventQueue q;

    // FINAL sentinel: fires at t = 2*(n+1) in algorithm units.
    // If reached, the search found no n-way overlap in that range.
    // For integer-speed runners the first lonely time is well within this bound.
    {
        EventPoint fp{};
        fp.number_of_runners = n;
        fp.rounds            = 1;
        fp.speed             = 1;
        fp.runner_number     = n + 1;         // sentinel index, above all runners
        fp.local_position    = n + 1;
        fp.type              = PointType::FINAL;
        NTL::ZZ num = NTL::to_ZZ(fp.local_position)
                    + NTL::to_ZZ(fp.rounds) * (n + 1);
        q.push({fp, num});
    }

    // Seed one interval per runner.
    // rounds = -1 so the first push_interval call yields rounds = 0 (k-th lap k=0).
    for (int i = 0; i < n; ++i) {
        EventPoint seed{};
        seed.number_of_runners = n;
        seed.rounds            = -1;
        seed.speed             = speeds[i];
        seed.runner_number     = i;
        seed.local_position    = 0;
        seed.type              = PointType::START;
        push_interval(seed, q, n);
    }

    int overlap = 0;
    while (!q.empty()) {
        auto [p, _] = q.top();
        q.pop();

        if (p.type == PointType::START) {
            if (++overlap == n) {
                return GeoResult{true, p};
            }
        } else if (p.type == PointType::END) {
            --overlap;
            push_interval(p, q, n);
        } else {
            // FINAL sentinel reached — no n-way overlap found in search range
            return std::nullopt;
        }
    }
    return std::nullopt;  // queue drained (should not happen before FINAL)
}
