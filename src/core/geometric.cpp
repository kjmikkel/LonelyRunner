#include "geometric.h"
#include <queue>
#include <vector>
#include <NTL/ZZ.h>
using namespace NTL;

struct CompareEvents {
    bool operator()(const EventPoint& a, const EventPoint& b) const {
        ZZ ta = (to_ZZ(a.local_position) + to_ZZ(a.rounds) * (a.number_of_runners + 1))
                * b.speed;
        ZZ tb = (to_ZZ(b.local_position) + to_ZZ(b.rounds) * (b.number_of_runners + 1))
                * a.speed;
        if (ta < tb) return false;
        if (ta > tb) return true;
        if (a.type == PointType::FINAL) return false;
        if (a.type == PointType::START && b.type == PointType::END) return false;
        if (a.runner_number < b.runner_number) return false;
        return true;
    }
};

using EventQueue = std::priority_queue<EventPoint,
                                        std::vector<EventPoint>,
                                        CompareEvents>;

static EventPoint make_point(const EventPoint& src, int position, PointType type) {
    EventPoint p = src;
    p.rounds         = src.rounds + 1;
    p.local_position = position;
    p.type           = type;
    return p;
}

static void push_interval(const EventPoint& end_pt, EventQueue& q, int length) {
    q.push(make_point(end_pt, 1,      PointType::START));
    q.push(make_point(end_pt, length, PointType::END));
}

std::optional<GeoResult> geometric_method(std::span<const int> speeds) {
    const int n = static_cast<int>(speeds.size());
    if (n < 1) return std::nullopt;   // Fix #1: was missing return

    EventQueue q;

    // Sentinel FINAL point
    EventPoint final_pt{};
    final_pt.number_of_runners = n;
    final_pt.rounds            = 1;
    final_pt.speed             = 1;
    final_pt.runner_number     = n + 1;
    final_pt.local_position    = n + 1;
    final_pt.type              = PointType::FINAL;
    q.push(final_pt);

    // Seed one interval per runner (rounds=-1 so first push_interval yields rounds=0)
    for (int i = 0; i < n; ++i) {
        EventPoint seed{};
        seed.number_of_runners = n;
        seed.rounds            = -1;   // Fix #4: field is now int, so -1 is valid
        seed.speed             = speeds[i];
        seed.runner_number     = i;
        seed.local_position    = 0;
        seed.type              = PointType::START;
        push_interval(seed, q, n);
    }

    int overlap = 0;
    while (!q.empty()) {
        EventPoint p = q.top(); q.pop();

        if (p.type == PointType::START) {
            if (++overlap == n) {
                GeoResult r;
                r.found = true;
                r.point = p;
                return r;
            }
        } else if (p.type == PointType::END) {
            --overlap;
            push_interval(p, q, n);
        } else {
            return std::nullopt;   // Fix #2: FINAL reached — no solution in this range
        }
    }
    return std::nullopt;           // Fix #3: queue drained — no return in original (UB)
}
