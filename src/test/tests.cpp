#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <atomic>
#include "numerical.h"
#include "geometric.h"
#include "prime.h"
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

    std::cout << g_pass << "/" << g_run << " tests passed\n";
    return (g_pass == g_run) ? 0 : 1;
}
