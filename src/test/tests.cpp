#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <atomic>
#include "numerical.h"
#include "geometric.h"
#include "prime.h"
#include "prime_modular.h"
#include "util.h"
#include "range_test.h"

static int g_run = 0, g_pass = 0;

#define CHECK(expr) do { \
    ++g_run; \
    if (!(expr)) { std::cerr << "FAIL: " #expr " (" __FILE__ ":" << __LINE__ << ")\n"; } \
    else { ++g_pass; } \
} while(0)

// ── Task 3: check_for_solution and is_valid ──────────────────────────────────

void test_check_for_solution() {
    CHECK(!check_for_solution(std::vector<int>{2, 4, 6}));
    CHECK(check_for_solution(std::vector<int>{3, 5, 7}));
    CHECK(check_for_solution(std::vector<int>{1}));
}

void test_is_valid_num_known_good() {
    NumResult r{true, 1, 2, 1};
    CHECK(is_valid(r, std::vector<int>{1, 2}));
}

void test_is_valid_num_rejects_bad() {
    NumResult r{true, 1, 2, 0};
    CHECK(!is_valid(r, std::vector<int>{1, 2}));
}

// ── Task 4: Numerical_method ─────────────────────────────────────────────────

void test_numerical_single_runner() {
    auto r = numerical_method(std::vector<int>{5});
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, std::vector<int>{5}));
}

void test_numerical_two_runners() {
    std::vector<int> speeds{1, 2};
    auto r = numerical_method(speeds);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

void test_numerical_three_runners() {
    std::vector<int> speeds{2, 3, 4};
    CHECK(!check_for_solution(speeds));
    auto r = numerical_method(speeds);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

void test_numerical_candidate_k_updated() {
    std::vector<int> speeds{1, 3, 5};
    auto r = numerical_method(speeds, /*find_maximum=*/true);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

// ── Task 5: Geometric_method ─────────────────────────────────────────────────

void test_geometric_empty() {
    auto r = geometric_method(std::span<const int>{});
    CHECK(!r.has_value());
}

void test_geometric_one_runner() {
    std::vector<int> speeds{3};
    auto r = geometric_method(speeds);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

void test_geometric_two_runners() {
    std::vector<int> speeds{1, 2};
    auto r = geometric_method(speeds);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

void test_geometric_matches_numerical() {
    std::vector<int> speeds{3, 7, 11};
    auto gr = geometric_method(speeds);
    auto nr = numerical_method(speeds);
    CHECK(gr.has_value() == nr.has_value());
    if (gr) CHECK(is_valid(*gr, speeds));
    if (nr) CHECK(is_valid(*nr, speeds));
}

// ── Task 6: Prime sieve ──────────────────────────────────────────────────────

void test_prime_sieve_small() {
    auto primes = prime_sieve(10);
    CHECK((primes == std::vector<int>{2, 3, 5, 7}));
}

void test_prime_sieve_empty() {
    CHECK(prime_sieve(1).empty());
    CHECK(prime_sieve(0).empty());
}

void test_prime_sieve_count_to_100() {
    CHECK(prime_sieve(100).size() == 25);
}

// ── Task 7: File I/O ─────────────────────────────────────────────────────────

void test_read_speeds_from_json() {
    const char* path = "/tmp/lr_test_speeds.json";
    { std::ofstream f(path); f << "[3, 7, 11, 13]"; }
    auto speeds = read_speeds_from_json(path);
    CHECK((speeds == std::vector<int>{3, 7, 11, 13}));
}

// ── Task 8: Save/verify round-trip ──────────────────────────────────────────

void test_save_verify_round_trip() {
    const char* path = "/tmp/lr_test_roundtrip.json";
    std::vector<int> speeds{1, 2};
    auto nr = numerical_method(speeds);
    CHECK(nr.has_value());

    RangeResult rr;
    rr.status = RangeResult::Status::Clean;
    rr.speeds = speeds;
    save_result(path, rr, speeds, "numerical", nr->a, nr->k1 + nr->k2);
    CHECK(verify_result(path));
}

void test_verify_rejects_bad_time() {
    const char* path = "/tmp/lr_test_bad.json";
    std::vector<int> speeds{1, 2};
    RangeResult rr;
    rr.status = RangeResult::Status::Clean;
    rr.speeds = speeds;
    save_result(path, rr, speeds, "numerical", 0, 3);
    CHECK(!verify_result(path));
}

// ── Task 9: Range test ───────────────────────────────────────────────────────

void test_range_test_small_clean() {
    std::atomic<bool> cancel{false};
    int last_progress = 0;
    RangeConfig cfg;
    cfg.start_value     = 1;
    cfg.end_value       = 10;
    cfg.num_runners     = 2;
    cfg.start_max_value = 2;
    cfg.pre_test        = false;
    cfg.algorithm       = Algorithm::Geometric;

    RangeResult r = range_test_sequential(cfg, cancel,
        [&](int cur, int) { last_progress = cur; });
    CHECK(r.status == RangeResult::Status::Clean);
    CHECK(last_progress > 0);
}

void test_range_test_numerical_clean() {
    std::atomic<bool> cancel{false};
    RangeConfig cfg;
    cfg.start_value     = 1;
    cfg.end_value       = 15;
    cfg.num_runners     = 2;
    cfg.start_max_value = 2;
    cfg.pre_test        = false;
    cfg.algorithm       = Algorithm::Numerical;

    RangeResult r = range_test_sequential(cfg, cancel, [](int,int){});
    CHECK(r.status == RangeResult::Status::Clean);
}

// ── Additional geometric correctness tests ───────────────────────────────────

void test_geometric_three_runners() {
    // Conjecture proven for n <= 7; {1,2,3} has an easy pre-filter solution at t=1/4
    std::vector<int> speeds{1, 2, 3};
    auto r = geometric_method(speeds);
    CHECK(r.has_value() && r->found);
    if (r) CHECK(is_valid(*r, speeds));
}

void test_geometric_matches_numerical_more() {
    // Cross-validate on several 3-runner sets
    for (auto& speeds : std::vector<std::vector<int>>{{1,2,3},{2,5,7},{3,5,11}}) {
        auto gr = geometric_method(speeds);
        auto nr = numerical_method(speeds);
        CHECK(gr.has_value() == nr.has_value());
        if (gr) CHECK(is_valid(*gr, speeds));
        if (nr) CHECK(is_valid(*nr, speeds));
    }
}

// Bug 1: CompareEvents tiebreaker is inconsistent when END(smaller runner_number)
// and START(larger runner_number) occur at the same time.  The comparator returns
// false for both comp(END_i, START_j) and comp(START_j, END_i), violating strict
// weak ordering and producing undefined priority-queue behaviour.
//
// Concrete trigger: speeds {6, 3} (n=2).  At t_alg = 1/3, runner_0 (speed=6)
// exits the lonely zone at the same moment runner_1 (speed=3) enters it.  Just
// before that moment only runner_0 is in the zone (overlap=1).  With correct
// START-before-END ordering, overlap jumps to 2 = n and the solution is found.
// With the buggy ordering the END fires first: overlap drops to 0, then the
// START brings it to 1 — the solution is missed and the method returns nullopt.
//
// Note: the same speed pair in the other order, {3,6}, does NOT trigger the bug
// (the first solution has the larger-index runner exiting, which the original
// comparator handles correctly via the START/END short-circuit).
void test_geometric_bug1_start_end_ordering() {
    std::vector<int> speeds_fwd{3, 6};  // should pass with either comparator
    std::vector<int> speeds_rev{6, 3};  // triggers Bug 1 with original comparator
    auto r_fwd = geometric_method(speeds_fwd);
    auto r_rev = geometric_method(speeds_rev);
    CHECK(r_fwd.has_value()); if (r_fwd) CHECK(is_valid(*r_fwd, speeds_fwd));
    CHECK(r_rev.has_value()); if (r_rev) CHECK(is_valid(*r_rev, speeds_rev)); // RED before fix
}

// Exhaustive 2-runner ordering invariant: result must not depend on which runner
// gets index 0.  The conjecture is proven for n=2, so a solution must always exist.
void test_geometric_ordering_invariant() {
    for (int s0 = 1; s0 <= 8; ++s0) {
        for (int s1 = s0 + 1; s1 <= 8; ++s1) {
            std::vector<int> fwd{s0, s1};
            std::vector<int> rev{s1, s0};
            auto r_fwd = geometric_method(fwd);
            auto r_rev = geometric_method(rev);
            CHECK(r_fwd.has_value()); if (r_fwd) CHECK(is_valid(*r_fwd, fwd));
            CHECK(r_rev.has_value()); if (r_rev) CHECK(is_valid(*r_rev, rev)); // RED for buggy pairs
        }
    }
}

// ── Additional numerical correctness tests ────────────────────────────────────

void test_numerical_empty_input() {
    auto r = numerical_method(std::span<const int>{});
    CHECK(!r.has_value());
}

// Speed 0 is not a valid conjecture input (the runner never moves), but the
// library should not crash.  Returning nullopt is the correct safe behaviour.
void test_numerical_speed_zero() {
    auto r = numerical_method(std::vector<int>{0});
    // We don't CHECK has_value — just verify it doesn't crash/throw
    (void)r;
    CHECK(true);  // reaching here means no crash
}

void test_numerical_find_maximum_is_latest() {
    // find_maximum=true must return a time >= the default (first-found) time
    std::vector<int> speeds{1, 3, 5};
    auto r_first = numerical_method(speeds, false);
    auto r_max   = numerical_method(speeds, true);
    CHECK(r_first.has_value() && r_max.has_value());
    if (r_first && r_max) {
        // t_first = a_first / (k1_first + k2_first),  t_max = a_max / (k1_max + k2_max)
        // t_max >= t_first  ⟺  a_max * (k1_first+k2_first) >= a_first * (k1_max+k2_max)
        long denom_first = r_first->k1 + r_first->k2;
        long denom_max   = r_max->k1   + r_max->k2;
        CHECK((long)r_max->a * denom_first >= (long)r_first->a * denom_max);
        CHECK(is_valid(*r_max, speeds));
    }
}

void test_numerical_large_speeds_no_overflow() {
    // Speeds large enough that int addition of a pair would overflow if not guarded
    std::vector<int> speeds{100, 201};
    auto r = numerical_method(speeds);
    CHECK(r.has_value());
    if (r) CHECK(is_valid(*r, speeds));
}

// ── Additional range test correctness tests ───────────────────────────────────

// Bug 2: numerical_method returns nullopt when its denominator search fails to
// find a lonely time — not only when the conjecture is violated.  Treating
// nullopt as a violation produces false positives in the range test.
void test_range_numerical_no_false_violations() {
    std::atomic<bool> cancel{false};
    RangeConfig cfg;
    cfg.start_value     = 1;
    cfg.end_value       = 15;
    cfg.num_runners     = 3;
    cfg.start_max_value = 3;
    cfg.pre_test        = false;
    cfg.algorithm       = Algorithm::Numerical;  // Bug 2 path
    RangeResult r = range_test_sequential(cfg, cancel, [](int,int){});
    CHECK(r.status == RangeResult::Status::Clean);  // RED before fix
}

void test_range_test_cancel() {
    std::atomic<bool> cancel{true};  // pre-cancelled
    RangeConfig cfg;
    cfg.start_value     = 1;
    cfg.end_value       = 100;
    cfg.num_runners     = 3;
    cfg.start_max_value = 3;
    cfg.pre_test        = false;
    cfg.algorithm       = Algorithm::Geometric;
    RangeResult r = range_test_sequential(cfg, cancel, [](int,int){});
    CHECK(r.status == RangeResult::Status::Cancelled);
}

// ── Additional check_for_solution tests ──────────────────────────────────────

void test_check_for_solution_empty() {
    // No runners: no solution to find
    CHECK(!check_for_solution(std::span<const int>{}));
}

// ── Prime modular method (Rosenfeld / Trakulthongchai approach) ───────────────
//
// The prime modular method searches lonely times of the form t = a/((n+1)*p)
// for small primes p.  This denominator family is central to the 2025-2026
// proofs for 8-10 runners (arXiv:2509.14111, 2511.22427, 2512.01912).

void test_prime_modular_empty() {
    auto r = prime_modular_method(std::span<const int>{});
    CHECK(!r.has_value());
}

void test_prime_modular_one_runner() {
    // Single runner with speed 3: lonely at t = 1/(2*p) for any prime p.
    // With p=2, Q=4: a=2 → 2*3 mod 4 = 2, min(2,2)=2, 2*2=4 >= 4 ✓
    std::vector<int> speeds{3};
    auto r = prime_modular_method(speeds);
    CHECK(r.has_value() && r->found);
    if (r) CHECK(is_valid(*r, speeds));
}

void test_prime_modular_two_runners() {
    std::vector<int> speeds{1, 2};
    auto r = prime_modular_method(speeds);
    CHECK(r.has_value() && r->found);
    if (r) CHECK(is_valid(*r, speeds));
}

void test_prime_modular_three_runners() {
    std::vector<int> speeds{2, 3, 4};
    auto r = prime_modular_method(speeds);
    CHECK(r.has_value() && r->found);
    if (r) CHECK(is_valid(*r, speeds));
}

void test_prime_modular_is_valid_known_good() {
    // t = 2/6 for speeds {1,2}: (n+1)=3, p=2 → Q=6
    //   v=1: 2*1 mod 6=2, min(2,4)=2, 2*3=6>=6 ✓
    //   v=2: 2*2 mod 6=4, min(4,2)=2, 2*3=6>=6 ✓
    PrimeModResult r{true, 2, 2};
    CHECK(is_valid(r, std::vector<int>{1, 2}));
}

void test_prime_modular_is_valid_rejects_bad() {
    // a=1 with same configuration: v=1 → pos=1, min(1,5)=1, 1*3=3 < 6 ✗
    PrimeModResult r{true, 2, 1};
    CHECK(!is_valid(r, std::vector<int>{1, 2}));
}

void test_prime_modular_is_valid_not_found() {
    // found=false must always be rejected
    PrimeModResult r{false, 2, 2};
    CHECK(!is_valid(r, std::vector<int>{1, 2}));
}

// Cross-validate: prime modular and geometric must both succeed (or both not),
// and when they succeed their results must individually pass is_valid.
void test_prime_modular_agrees_with_geometric() {
    for (auto& speeds : std::vector<std::vector<int>>{
            {1, 2}, {3, 5}, {1, 2, 3}, {2, 3, 4}, {1, 3, 5}, {3, 7, 11}}) {
        auto gr = geometric_method(speeds);
        auto pr = prime_modular_method(speeds);
        // Geometric is the authoritative verifier — if it finds a solution,
        // one exists.  Prime modular may miss it (incomplete search), so we
        // only check that prime modular doesn't spuriously claim a solution
        // when none exists.
        if (pr) CHECK(is_valid(*pr, speeds));
        if (gr)  CHECK(is_valid(*gr, speeds));
        // For these small, well-studied inputs a prime solution must exist
        CHECK(pr.has_value());
    }
}

// Consistency: result from prime_modular_method must satisfy is_valid when found=true
void test_prime_modular_result_self_consistent() {
    for (int s0 = 1; s0 <= 6; ++s0) {
        for (int s1 = s0 + 1; s1 <= 6; ++s1) {
            std::vector<int> speeds{s0, s1};
            auto r = prime_modular_method(speeds);
            if (r && r->found) CHECK(is_valid(*r, speeds));
        }
    }
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    // Headless verification: LonelyRunnerTests --verify path/to/file.json
    if (argc == 3 && std::string(argv[1]) == "--verify") {
        bool ok = verify_result(argv[2]);
        std::cout << (ok ? "PASS: result is valid\n" : "FAIL: result is invalid\n");
        return ok ? 0 : 1;
    }

    test_check_for_solution();
    test_is_valid_num_known_good();
    test_is_valid_num_rejects_bad();
    test_numerical_single_runner();
    test_numerical_two_runners();
    test_numerical_three_runners();
    test_numerical_candidate_k_updated();
    test_geometric_empty();
    test_geometric_one_runner();
    test_geometric_two_runners();
    test_geometric_matches_numerical();
    test_prime_sieve_small();
    test_prime_sieve_empty();
    test_prime_sieve_count_to_100();
    test_read_speeds_from_json();
    test_save_verify_round_trip();
    test_verify_rejects_bad_time();
    test_range_test_small_clean();
    test_range_test_numerical_clean();
    test_geometric_three_runners();
    test_geometric_matches_numerical_more();
    test_geometric_bug1_start_end_ordering();
    test_geometric_ordering_invariant();
    test_numerical_empty_input();
    test_numerical_speed_zero();
    test_numerical_find_maximum_is_latest();
    test_numerical_large_speeds_no_overflow();
    test_range_numerical_no_false_violations();
    test_range_test_cancel();
    test_check_for_solution_empty();

    test_prime_modular_empty();
    test_prime_modular_one_runner();
    test_prime_modular_two_runners();
    test_prime_modular_three_runners();
    test_prime_modular_is_valid_known_good();
    test_prime_modular_is_valid_rejects_bad();
    test_prime_modular_is_valid_not_found();
    test_prime_modular_agrees_with_geometric();
    test_prime_modular_result_self_consistent();

    std::cout << g_pass << "/" << g_run << " tests passed\n";
    return (g_pass == g_run) ? 0 : 1;
}
