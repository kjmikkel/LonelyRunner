# Lonely Runner Verifier — Modernization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Modernize the Lonely Runner Conjecture Verifier from a GTK+2 C++03 application into a clean C++20 Linux application with Qt6 GUI, parallel range test, and runner animation.

**Architecture:** Three CMake targets — `liblonelyrunner` (core algorithms, no Qt), `LonelyRunnerApp` (Qt6 GUI), and `LonelyRunnerTests` (test harness). The core library is Qt-free; `rangeworker.cpp` in the app bridges it to `QtConcurrent`. All raw pointers and `new`/`delete` replaced with `std::optional`, `std::vector`, and value semantics. Old `src/C++/` is left untouched as an archive — nothing in it is compiled.

**Tech Stack:** C++20, Qt6 (Widgets, Concurrent), NTL + GMP, nlohmann/json (header-only, bundled), CMake 3.20, Ninja

---

## File Map

**Created:**
- `CMakeLists.txt` — root, finds Qt6/NTL/GMP, enables testing
- `src/core/CMakeLists.txt` — static lib target
- `src/core/data_structures.h` — all shared types (EventPoint, GeoResult, NumResult, RangeConfig, RangeResult)
- `src/core/geometric.h` / `src/core/geometric.cpp` — fixed Geometric_method
- `src/core/numerical.h` / `src/core/numerical.cpp` — fixed Numerical_method, is_valid, check_for_solution
- `src/core/prime.h` / `src/core/prime.cpp` — prime sieve
- `src/core/util.h` / `src/core/util.cpp` — JSON file I/O, save/load/verify result
- `src/core/range_test.h` / `src/core/range_test.cpp` — combinatorial logic, sequential range test
- `src/app/CMakeLists.txt` — Qt executable target
- `src/app/main.cpp` — Qt app entry point
- `src/app/mainwindow.h` / `src/app/mainwindow.cpp` — sidebar nav, QStackedWidget
- `src/app/manualtestpanel.h` / `src/app/manualtestpanel.cpp` — manual test panel
- `src/app/rangetestpanel.h` / `src/app/rangetestpanel.cpp` — range test panel
- `src/app/verifyfilepanel.h` / `src/app/verifyfilepanel.cpp` — verify file panel
- `src/app/rangeworker.h` / `src/app/rangeworker.cpp` — QThread + QtConcurrent wrapper
- `src/app/animationwidget.h` / `src/app/animationwidget.cpp` — floating animation window
- `src/test/CMakeLists.txt` — test executable, ctest registration
- `src/test/tests.cpp` — all test cases
- `third_party/nlohmann/json.hpp` — bundled single header

**Modified:**
- `.gitignore` — add `build/`, `.superpowers/`

---

## Task 1: Project skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/core/CMakeLists.txt`
- Create: `src/app/CMakeLists.txt`
- Create: `src/test/CMakeLists.txt`
- Modify: `.gitignore`

- [ ] **Step 1: Install dependencies**

```bash
sudo apt install libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build
```

Expected: packages install without error.

- [ ] **Step 2: Download nlohmann/json**

```bash
mkdir -p third_party/nlohmann
curl -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
     -o third_party/nlohmann/json.hpp
```

Expected: `third_party/nlohmann/json.hpp` exists and is ~900KB.

- [ ] **Step 3: Create directory structure**

```bash
mkdir -p src/core src/app src/test data
```

- [ ] **Step 4: Write root CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(LonelyRunner CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

find_library(NTL_LIB NAMES ntl REQUIRED)
find_library(GMP_LIB NAMES gmp REQUIRED)
find_path(NTL_INCLUDE_DIR NAMES NTL/ZZ.h REQUIRED)

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(src/test)
```

- [ ] **Step 5: Write src/core/CMakeLists.txt**

```cmake
add_library(lonelyrunner STATIC
    geometric.cpp
    numerical.cpp
    prime.cpp
    util.cpp
    range_test.cpp
)

target_include_directories(lonelyrunner
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/third_party
    PRIVATE ${NTL_INCLUDE_DIR}
)

target_link_libraries(lonelyrunner PUBLIC ${NTL_LIB} ${GMP_LIB} m)
target_compile_options(lonelyrunner PRIVATE -O2 -Wall -Wextra)
```

- [ ] **Step 6: Write src/app/CMakeLists.txt**

```cmake
qt6_add_executable(LonelyRunnerApp
    main.cpp
    mainwindow.cpp
    manualtestpanel.cpp
    rangetestpanel.cpp
    verifyfilepanel.cpp
    rangeworker.cpp
    animationwidget.cpp
)

target_link_libraries(LonelyRunnerApp PRIVATE
    lonelyrunner
    Qt6::Widgets
    Qt6::Concurrent
)
```

- [ ] **Step 7: Write src/test/CMakeLists.txt**

```cmake
add_executable(LonelyRunnerTests tests.cpp)
target_link_libraries(LonelyRunnerTests PRIVATE lonelyrunner)
add_test(NAME unit_tests COMMAND LonelyRunnerTests)
```

- [ ] **Step 8: Create empty stub source files so CMake can configure**

```bash
touch src/core/geometric.cpp  src/core/geometric.h
touch src/core/numerical.cpp  src/core/numerical.h
touch src/core/prime.cpp      src/core/prime.h
touch src/core/util.cpp       src/core/util.h
touch src/core/range_test.cpp src/core/range_test.h
touch src/core/data_structures.h
touch src/app/main.cpp        src/app/mainwindow.cpp    src/app/mainwindow.h
touch src/app/manualtestpanel.cpp src/app/manualtestpanel.h
touch src/app/rangetestpanel.cpp  src/app/rangetestpanel.h
touch src/app/verifyfilepanel.cpp src/app/verifyfilepanel.h
touch src/app/rangeworker.cpp src/app/rangeworker.h
touch src/app/animationwidget.cpp src/app/animationwidget.h
touch src/test/tests.cpp
```

- [ ] **Step 9: Verify CMake configures without error**

```bash
cmake -B build -G Ninja
```

Expected: `-- Build files have been written to: .../build` with no errors.

- [ ] **Step 10: Update .gitignore**

Add to `.gitignore`:
```
build/
.superpowers/
```

- [ ] **Step 11: Commit**

```bash
git add CMakeLists.txt src/core/CMakeLists.txt src/app/CMakeLists.txt \
        src/test/CMakeLists.txt third_party/ .gitignore \
        src/core/ src/app/ src/test/
git commit -m "feat: add CMake project skeleton with three targets"
```

---

## Task 2: data_structures.h

**Files:**
- Create: `src/core/data_structures.h`

- [ ] **Step 1: Write data_structures.h**

```cpp
#pragma once
#include <vector>
#include <chrono>
#include <string>

enum class PointType { START, END, FINAL };

struct EventPoint {
    int local_position{};
    int number_of_runners{};
    int rounds{};           // int (not unsigned) — fixes signed/unsigned bug #4
    int speed{};
    int runner_number{};
    PointType type{PointType::START};
};

struct GeoResult {
    bool found{false};
    EventPoint point{};
};

struct NumResult {
    bool found{false};
    int k1{}, k2{}, a{};
};

enum class Algorithm { Geometric, Numerical };

struct RangeConfig {
    int start_value{1};
    int end_value{100};
    int num_runners{3};
    int start_max_value{3};
    bool pre_test{false};
    Algorithm algorithm{Algorithm::Geometric};
};

struct RangeResult {
    enum class Status { Clean, ViolationFound, Cancelled };
    Status status{Status::Clean};
    std::vector<int> speeds{};
    std::chrono::microseconds elapsed{};
};
```

- [ ] **Step 2: Verify it compiles**

```bash
cmake --build build --target lonelyrunner 2>&1 | head -20
```

Expected: errors only from empty .cpp stubs — not from data_structures.h itself.

- [ ] **Step 3: Commit**

```bash
git add src/core/data_structures.h
git commit -m "feat: add modernized data_structures.h"
```

---

## Task 3: Numerical utilities — check_for_solution and is_valid

**Files:**
- Create: `src/core/numerical.h`
- Create: `src/core/numerical.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/tests.cpp
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include "numerical.h"

static int g_run = 0, g_pass = 0;

#define CHECK(expr) do { \
    ++g_run; \
    if (!(expr)) { std::cerr << "FAIL: " #expr " (" __FILE__ ":" << __LINE__ << ")\n"; } \
    else { ++g_pass; } \
} while(0)

void test_check_for_solution() {
    // {2,4,6}: every number in {2,3,4} divides at least one speed — no trivial solution
    CHECK(!check_for_solution(std::vector<int>{2, 4, 6}));
    // {3,5,7}: 2 divides none of them — trivial solution exists
    CHECK(check_for_solution(std::vector<int>{3, 5, 7}));
    // {1}: 2 doesn't divide 1 — trivial solution
    CHECK(check_for_solution(std::vector<int>{1}));
}

void test_is_valid_num_known_good() {
    // speeds {1,2}: k1=1, k2=2, a=1 => t=1/3
    // runner 1 at pos 1/3, dist=1/3 >= 1/3 ok
    // runner 2 at pos 2/3, dist=1/3 >= 1/3 ok
    NumResult r{true, 1, 2, 1};
    CHECK(is_valid(r, std::vector<int>{1, 2}));
}

void test_is_valid_num_rejects_bad() {
    // t=0: all runners at start — trivially invalid
    NumResult r{true, 1, 2, 0};
    CHECK(!is_valid(r, std::vector<int>{1, 2}));
}

int main(int argc, char* argv[]) {
    test_check_for_solution();
    test_is_valid_num_known_good();
    test_is_valid_num_rejects_bad();
    std::cout << g_pass << "/" << g_run << " tests passed\n";
    return (g_pass == g_run) ? 0 : 1;
}
```

- [ ] **Step 2: Run to verify tests fail**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: compilation error — `check_for_solution` not defined.

- [ ] **Step 3: Write numerical.h**

```cpp
#pragma once
#include "data_structures.h"
#include <optional>
#include <span>

std::optional<NumResult> numerical_method(std::span<const int> speeds,
                                          bool find_maximum = false);
bool is_valid(const NumResult& result, std::span<const int> speeds);
bool is_valid(const GeoResult& result, std::span<const int> speeds);
bool check_for_solution(std::span<const int> speeds);
```

- [ ] **Step 4: Write numerical.cpp — utilities only (numerical_method is a stub)**

```cpp
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

// Stub — replaced in Task 4
std::optional<NumResult> numerical_method(std::span<const int>, bool) {
    return std::nullopt;
}
```

- [ ] **Step 5: Run tests — should all pass**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `3/3 tests passed`

- [ ] **Step 6: Commit**

```bash
git add src/core/numerical.h src/core/numerical.cpp src/test/tests.cpp
git commit -m "feat: add check_for_solution and is_valid with bug fixes #5 #6"
```

---

## Task 4: Numerical_method

**Files:**
- Modify: `src/core/numerical.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add failing tests — append these functions and add calls in main()**

```cpp
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
    CHECK(!check_for_solution(speeds));  // no trivial solution
    auto r = numerical_method(speeds);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}

void test_numerical_candidate_k_updated() {
    // Bug #8: candidate_k was never updated — wrong comparison on 2nd+ candidates.
    // This set has multiple valid (fi,si) pairs; with the bug the best is wrong.
    std::vector<int> speeds{1, 3, 5};
    auto r = numerical_method(speeds, /*find_maximum=*/true);
    CHECK(r.has_value() && r->found);
    CHECK(is_valid(*r, speeds));
}
```

In `main()`, add before the final return:
```cpp
test_numerical_single_runner();
test_numerical_two_runners();
test_numerical_three_runners();
test_numerical_candidate_k_updated();
```

- [ ] **Step 2: Run — verify new tests fail**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `3/7 tests passed`

- [ ] **Step 3: Replace the stub in numerical.cpp with full implementation**

Replace the stub `numerical_method` at the bottom of `numerical.cpp`:

```cpp
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
```

- [ ] **Step 4: Run tests — all should pass**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `7/7 tests passed`

- [ ] **Step 5: Commit**

```bash
git add src/core/numerical.cpp src/test/tests.cpp
git commit -m "feat: implement Numerical_method with bug fixes #7 #8"
```

---

## Task 5: Geometric_method

**Files:**
- Modify: `src/core/geometric.h`
- Modify: `src/core/geometric.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add failing tests — append and call from main()**

```cpp
void test_geometric_empty() {
    // Fix #1: original code missing return — fell through to queue allocation
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
    // Both algorithms must find a valid solution for this set
    std::vector<int> speeds{3, 7, 11};
    auto gr = geometric_method(speeds);
    auto nr = numerical_method(speeds);
    CHECK(gr.has_value() == nr.has_value());
    if (gr) CHECK(is_valid(*gr, speeds));
    if (nr) CHECK(is_valid(*nr, speeds));
}
```

- [ ] **Step 2: Run — verify fail**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: compilation error — `geometric_method` not defined.

- [ ] **Step 3: Write geometric.h**

```cpp
#pragma once
#include "data_structures.h"
#include <optional>
#include <span>

std::optional<GeoResult> geometric_method(std::span<const int> speeds);
```

- [ ] **Step 4: Write geometric.cpp**

```cpp
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
```

- [ ] **Step 5: Run tests**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `11/11 tests passed`

- [ ] **Step 6: Commit**

```bash
git add src/core/geometric.h src/core/geometric.cpp src/test/tests.cpp
git commit -m "feat: implement Geometric_method with bug fixes #1 #2 #3 #4"
```

---

## Task 6: Prime sieve

**Files:**
- Modify: `src/core/prime.h`
- Modify: `src/core/prime.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add failing tests — append and call from main()**

```cpp
void test_prime_sieve_small() {
    auto primes = prime_sieve(10);
    CHECK((primes == std::vector<int>{2, 3, 5, 7}));
}

void test_prime_sieve_empty() {
    CHECK(prime_sieve(1).empty());
    CHECK(prime_sieve(0).empty());
}

void test_prime_sieve_count_to_100() {
    // There are exactly 25 primes <= 100
    CHECK(prime_sieve(100).size() == 25);
}
```

- [ ] **Step 2: Run — verify fail**

Expected: compilation error — `prime_sieve` not defined.

- [ ] **Step 3: Write prime.h**

```cpp
#pragma once
#include <vector>

std::vector<int> prime_sieve(int up_to);
```

- [ ] **Step 4: Write prime.cpp**

```cpp
#include "prime.h"
#include <numeric>

std::vector<int> prime_sieve(int up_to) {
    if (up_to < 2) return {};

    std::vector<int> sieve(up_to + 1);
    std::iota(sieve.begin(), sieve.end(), 0);
    sieve[0] = sieve[1] = 0;

    for (int f = 2; f * f <= up_to; ++f) {
        if (sieve[f] == 0) continue;
        for (int m = f + f; m <= up_to; m += f)
            sieve[m] = 0;
    }

    std::vector<int> primes;
    for (int i = 2; i <= up_to; ++i)
        if (sieve[i] != 0) primes.push_back(i);
    return primes;
}
```

- [ ] **Step 5: Run tests**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `14/14 tests passed`

- [ ] **Step 6: Commit**

```bash
git add src/core/prime.h src/core/prime.cpp src/test/tests.cpp
git commit -m "feat: add prime_sieve"
```

---

## Task 7: File I/O — read speeds from JSON

**Files:**
- Modify: `src/core/util.h`
- Modify: `src/core/util.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add failing test — append and call from main()**

```cpp
void test_read_speeds_from_json() {
    const char* path = "/tmp/lr_test_speeds.json";
    { std::ofstream f(path); f << "[3, 7, 11, 13]"; }
    auto speeds = read_speeds_from_json(path);
    CHECK((speeds == std::vector<int>{3, 7, 11, 13}));
}
```

- [ ] **Step 2: Run — verify fail**

Expected: compilation error — `read_speeds_from_json` not defined.

- [ ] **Step 3: Write util.h**

```cpp
#pragma once
#include "data_structures.h"
#include <filesystem>
#include <vector>
#include <string>

std::vector<int> read_speeds_from_json(const std::filesystem::path& path);

void save_result(const std::filesystem::path& path,
                 const RangeResult& result,
                 const std::vector<int>& speeds,
                 const std::string& algorithm,
                 int numerator, int denominator);

bool verify_result(const std::filesystem::path& path);
```

- [ ] **Step 4: Write util.cpp**

```cpp
#include "util.h"
#include "numerical.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <ctime>
using json = nlohmann::json;

std::vector<int> read_speeds_from_json(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    json j = json::parse(f);
    return j.get<std::vector<int>>();
}

void save_result(const std::filesystem::path& path,
                 const RangeResult& result,
                 const std::vector<int>& speeds,
                 const std::string& algorithm,
                 int numerator, int denominator) {
    json j;
    j["version"]      = 1;
    j["type"]         = (result.status == RangeResult::Status::ViolationFound)
                        ? "violation_candidate" : "solution";
    j["speeds"]       = speeds;
    j["num_runners"]  = static_cast<int>(speeds.size());
    j["algorithm"]    = algorithm;
    j["result_found"] = (result.status != RangeResult::Status::ViolationFound);
    j["time"]         = {{"numerator", numerator}, {"denominator", denominator}};

    auto now  = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    j["timestamp"] = buf;

    std::ofstream out(path);
    if (!out) throw std::runtime_error("Cannot write: " + path.string());
    out << j.dump(2);
}

bool verify_result(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    json j = json::parse(f);

    std::vector<int> speeds = j["speeds"].get<std::vector<int>>();
    int num = j["time"]["numerator"];
    int den = j["time"]["denominator"];
    if (den == 0) return false;

    // Reconstruct a NumResult using k1=den, k2=0 for is_valid (canonical form)
    NumResult nr;
    nr.found = true;
    nr.a     = num;
    nr.k1    = den;
    nr.k2    = 0;
    return is_valid(nr, speeds);
}
```

- [ ] **Step 5: Run tests**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `15/15 tests passed`

- [ ] **Step 6: Commit**

```bash
git add src/core/util.h src/core/util.cpp src/test/tests.cpp
git commit -m "feat: add JSON file I/O with nlohmann/json replacing libjson-c"
```

---

## Task 8: Save/load round-trip and verify_result tests

**Files:**
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add tests — append and call from main()**

```cpp
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
    // a=0, denominator=3 => t=0 => all runners at start => invalid
    save_result(path, rr, speeds, "numerical", 0, 3);
    CHECK(!verify_result(path));
}
```

- [ ] **Step 2: Run tests**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `17/17 tests passed`

- [ ] **Step 3: Commit**

```bash
git add src/test/tests.cpp
git commit -m "test: add save/verify round-trip tests"
```

---

## Task 9: Range test core (sequential)

**Files:**
- Modify: `src/core/range_test.h`
- Modify: `src/core/range_test.cpp`
- Modify: `src/test/tests.cpp`

- [ ] **Step 1: Add failing tests — append and call from main()**

```cpp
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
```

- [ ] **Step 2: Run — verify fail**

Expected: compilation error — `range_test_sequential` not defined.

- [ ] **Step 3: Write range_test.h**

```cpp
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
```

- [ ] **Step 4: Write range_test.cpp**

```cpp
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
```

- [ ] **Step 5: Add --verify CLI path to test main()**

Replace `main()` in `src/test/tests.cpp` with:

```cpp
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
```

Add `#include "util.h"` and `#include "range_test.h"` at the top of tests.cpp.

- [ ] **Step 6: Run tests**

```bash
cmake --build build --target LonelyRunnerTests && \
  ./build/src/test/LonelyRunnerTests
```

Expected: `19/19 tests passed`

- [ ] **Step 7: Run ctest**

```bash
ctest --test-dir build --output-on-failure
```

Expected: `1/1 Test #1: unit_tests ... Passed`

- [ ] **Step 8: Commit**

```bash
git add src/core/range_test.h src/core/range_test.cpp src/test/tests.cpp
git commit -m "feat: add sequential range_test with bug fixes #9 #10 #11 #12 #13"
```

---

## Task 10: RangeWorker — Qt parallel wrapper

**Files:**
- Modify: `src/app/rangeworker.h`
- Modify: `src/app/rangeworker.cpp`

- [ ] **Step 1: Write rangeworker.h**

```cpp
#pragma once
#include <QObject>
#include <QFutureWatcher>
#include <atomic>
#include "range_test.h"

class RangeWorker : public QObject {
    Q_OBJECT
public:
    explicit RangeWorker(RangeConfig config, QObject* parent = nullptr);
    void start();
    void cancel();

signals:
    void progress(int current, int total, QString status);
    void finished(RangeResult result);

private:
    RangeConfig m_config;
    std::atomic<bool> m_cancel{false};
    QFutureWatcher<RangeResult> m_watcher;
};
```

- [ ] **Step 2: Write rangeworker.cpp**

```cpp
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
```

- [ ] **Step 3: Verify app still builds**

```bash
cmake --build build --target LonelyRunnerApp 2>&1 | tail -5
```

Expected: no linker errors (empty main.cpp is still fine).

- [ ] **Step 4: Commit**

```bash
git add src/app/rangeworker.h src/app/rangeworker.cpp
git commit -m "feat: add RangeWorker with QtConcurrent parallel range test"
```

---

## Task 11: MainWindow skeleton

**Files:**
- Modify: `src/app/main.cpp`
- Modify: `src/app/mainwindow.h`
- Modify: `src/app/mainwindow.cpp`

- [ ] **Step 1: Write main.cpp**

```cpp
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Lonely Runner Verifier");
    app.setApplicationVersion("2.0");
    MainWindow w;
    w.show();
    return app.exec();
}
```

- [ ] **Step 2: Write mainwindow.h**

```cpp
#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <vector>

class ManualTestPanel;
class RangeTestPanel;
class VerifyFilePanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void addToHistory(const std::vector<int>& speeds, bool violation);

private slots:
    void onNavSelected(int row);

private:
    QListWidget*     m_nav{};
    QStackedWidget*  m_stack{};
    QListWidget*     m_history{};
    ManualTestPanel* m_manualPanel{};
    RangeTestPanel*  m_rangePanel{};
    VerifyFilePanel* m_verifyPanel{};

    void buildLayout();
};
```

- [ ] **Step 3: Write mainwindow.cpp**

```cpp
#include "mainwindow.h"
#include "manualtestpanel.h"
#include "rangetestpanel.h"
#include "verifyfilepanel.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Lonely Runner Verifier");
    resize(920, 620);
    buildLayout();
}

void MainWindow::buildLayout() {
    m_nav = new QListWidget;
    m_nav->addItem("Manual Test");
    m_nav->addItem("Range Test");
    m_nav->addItem("Animation");
    m_nav->addItem("Verify File");
    m_nav->setFixedWidth(145);
    m_nav->setCurrentRow(0);

    m_history = new QListWidget;
    m_history->setFixedWidth(145);

    auto* sidebar       = new QWidget;
    auto* sideLayout    = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(4, 4, 4, 4);
    sideLayout->addWidget(m_nav, 3);
    sideLayout->addWidget(new QLabel("History"), 0);
    sideLayout->addWidget(m_history, 1);

    m_manualPanel = new ManualTestPanel(this);
    m_rangePanel  = new RangeTestPanel(this);
    m_verifyPanel = new VerifyFilePanel(this);

    m_stack = new QStackedWidget;
    m_stack->addWidget(m_manualPanel);   // 0
    m_stack->addWidget(m_rangePanel);    // 1
    m_stack->addWidget(m_verifyPanel);   // 2

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(sidebar);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    connect(m_nav, &QListWidget::currentRowChanged,
            this,  &MainWindow::onNavSelected);

    connect(m_manualPanel, &ManualTestPanel::resultReady,
            this, [this](std::vector<int> speeds, bool valid, int, int) {
        addToHistory(speeds, !valid);
    });

    connect(m_rangePanel, &RangeTestPanel::resultReady,
            this, [this](std::vector<int> speeds, bool violation, int, int) {
        addToHistory(speeds, violation);
    });

    connect(m_history, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
        // Strip "✓ " or "⚠ " prefix, restore to manual test input
        QString label = item->text().mid(2);
        m_nav->setCurrentRow(0);
        m_manualPanel->setSpeedText(label);
    });
}

void MainWindow::onNavSelected(int row) {
    if (row == 2) {
        // Animation has no embedded panel — open via "Animate this" buttons
        QMessageBox::information(this, "Animation",
            "Run a Manual Test or Range Test first,\n"
            "then click \"Animate this\" to open the animation window.");
        m_nav->setCurrentRow(m_stack->currentIndex());
        return;
    }
    int stackIndex = (row < 2) ? row : row - 1;  // skip animation slot
    m_stack->setCurrentIndex(stackIndex);
}

void MainWindow::addToHistory(const std::vector<int>& speeds, bool violation) {
    QString label = violation ? "⚠ " : "✓ ";
    int show = static_cast<int>(std::min(speeds.size(), size_t(4)));
    for (int i = 0; i < show; ++i) {
        if (i > 0) label += ",";
        label += QString::number(speeds[i]);
    }
    if (speeds.size() > 4) label += "…";
    m_history->insertItem(0, label);
}
```

- [ ] **Step 4: Build and run**

```bash
cmake --build build --target LonelyRunnerApp && \
  ./build/src/app/LonelyRunnerApp
```

Expected: window opens with sidebar (Manual Test, Range Test, Animation, Verify File) and placeholder panels. Clicking Animation shows the informational dialog.

- [ ] **Step 5: Commit**

```bash
git add src/app/main.cpp src/app/mainwindow.h src/app/mainwindow.cpp
git commit -m "feat: add MainWindow skeleton with sidebar nav and history list"
```

---

## Task 12: Manual Test panel

**Files:**
- Modify: `src/app/manualtestpanel.h`
- Modify: `src/app/manualtestpanel.cpp`

- [ ] **Step 1: Write manualtestpanel.h**

```cpp
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <vector>

class ManualTestPanel : public QWidget {
    Q_OBJECT
public:
    explicit ManualTestPanel(QWidget* parent = nullptr);
    void setSpeedText(const QString& text);

signals:
    void resultReady(std::vector<int> speeds, bool valid, int numerator, int denominator);

private slots:
    void onRun();
    void onImport();
    void onAnimate();

private:
    QLineEdit*    m_speedsInput{};
    QCheckBox*    m_preTestBox{};
    QCheckBox*    m_findMaxBox{};
    QRadioButton* m_geoRadio{};
    QRadioButton* m_numRadio{};
    QLabel*       m_resultLabel{};
    QPushButton*  m_animateBtn{};

    std::vector<int> m_lastSpeeds;
    int m_lastNumerator{0}, m_lastDenominator{0};

    std::vector<int> parseSpeeds() const;
    void showResult(const QString& text, bool ok);
};
```

- [ ] **Step 2: Write manualtestpanel.cpp**

```cpp
#include "manualtestpanel.h"
#include "animationwidget.h"
#include "geometric.h"
#include "numerical.h"
#include "util.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

ManualTestPanel::ManualTestPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* inputGroup  = new QGroupBox("Runner Speeds");
    auto* inputLayout = new QHBoxLayout(inputGroup);
    m_speedsInput = new QLineEdit("3, 7, 11, 13");
    auto* importBtn = new QPushButton("Import JSON");
    inputLayout->addWidget(m_speedsInput);
    inputLayout->addWidget(importBtn);
    layout->addWidget(inputGroup);

    auto* optBox    = new QGroupBox("Options");
    auto* optLayout = new QHBoxLayout(optBox);
    m_preTestBox = new QCheckBox("Pre-test filter");
    m_findMaxBox = new QCheckBox("Find maximum t");
    optLayout->addWidget(m_preTestBox);
    optLayout->addWidget(m_findMaxBox);
    optLayout->addStretch();
    layout->addWidget(optBox);

    auto* algoBox    = new QGroupBox("Algorithm");
    auto* algoLayout = new QHBoxLayout(algoBox);
    m_geoRadio = new QRadioButton("Geometric");
    m_numRadio = new QRadioButton("Numerical");
    m_geoRadio->setChecked(true);
    algoLayout->addWidget(m_geoRadio);
    algoLayout->addWidget(m_numRadio);
    algoLayout->addStretch();
    layout->addWidget(algoBox);

    m_resultLabel = new QLabel("No result yet.");
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setStyleSheet(
        "QLabel{background:#1e1e2e;color:#cdd6f4;padding:8px;border-radius:4px;}");
    layout->addWidget(m_resultLabel, 1);

    auto* btnRow  = new QHBoxLayout;
    auto* runBtn  = new QPushButton("Run");
    runBtn->setDefault(true);
    m_animateBtn  = new QPushButton("Animate this");
    m_animateBtn->setEnabled(false);
    btnRow->addWidget(runBtn, 2);
    btnRow->addWidget(m_animateBtn, 1);
    layout->addLayout(btnRow);

    connect(runBtn,       &QPushButton::clicked, this, &ManualTestPanel::onRun);
    connect(importBtn,    &QPushButton::clicked, this, &ManualTestPanel::onImport);
    connect(m_animateBtn, &QPushButton::clicked, this, &ManualTestPanel::onAnimate);
}

void ManualTestPanel::setSpeedText(const QString& text) {
    m_speedsInput->setText(text);
}

std::vector<int> ManualTestPanel::parseSpeeds() const {
    std::vector<int> v;
    for (const QString& part : m_speedsInput->text().split(',')) {
        bool ok; int n = part.trimmed().toInt(&ok);
        if (ok && n > 0) v.push_back(n);
    }
    return v;
}

void ManualTestPanel::onRun() {
    m_lastSpeeds = parseSpeeds();
    if (m_lastSpeeds.empty()) {
        showResult("Enter at least one positive integer speed.", false);
        return;
    }

    if (m_preTestBox->isChecked() && check_for_solution(m_lastSpeeds)) {
        showResult("Pre-test: trivial solution detected.", true);
        m_animateBtn->setEnabled(false);
        return;
    }

    bool find_max = m_findMaxBox->isChecked();
    QString algo_name;
    bool valid = false;
    m_lastNumerator = m_lastDenominator = 0;

    if (m_geoRadio->isChecked()) {
        algo_name = "Geometric";
        auto r = geometric_method(m_lastSpeeds);
        if (r && r->found) {
            valid = is_valid(*r, m_lastSpeeds);
            const auto& p   = r->point;
            m_lastNumerator   = p.local_position + p.rounds * (p.number_of_runners + 1);
            m_lastDenominator = p.speed * (p.number_of_runners + 1);
        }
    } else {
        algo_name = "Numerical";
        auto r = numerical_method(m_lastSpeeds, find_max);
        if (r && r->found) {
            valid = is_valid(*r, m_lastSpeeds);
            m_lastNumerator   = r->a;
            m_lastDenominator = r->k1 + r->k2;
        }
    }

    if (valid) {
        showResult(QString("%1: Solution found \xe2\x9c\x93  t = %2 / %3")
                   .arg(algo_name).arg(m_lastNumerator).arg(m_lastDenominator), true);
        m_animateBtn->setEnabled(true);
        emit resultReady(m_lastSpeeds, true, m_lastNumerator, m_lastDenominator);
    } else {
        showResult(QString("%1: No valid solution found.").arg(algo_name), false);
        m_animateBtn->setEnabled(false);
        emit resultReady(m_lastSpeeds, false, 0, 0);
    }
}

void ManualTestPanel::onImport() {
    QString path = QFileDialog::getOpenFileName(this, "Open JSON", "", "JSON (*.json)");
    if (path.isEmpty()) return;
    try {
        auto speeds = read_speeds_from_json(path.toStdString());
        QString text;
        for (int i = 0; i < (int)speeds.size(); ++i) {
            if (i > 0) text += ", ";
            text += QString::number(speeds[i]);
        }
        m_speedsInput->setText(text);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Import failed", e.what());
    }
}

void ManualTestPanel::onAnimate() {
    auto* w = new AnimationWidget(m_lastSpeeds, m_lastNumerator, m_lastDenominator);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void ManualTestPanel::showResult(const QString& text, bool ok) {
    m_resultLabel->setText(text);
    m_resultLabel->setStyleSheet(ok
        ? "QLabel{background:#1e1e2e;color:#a6e3a1;padding:8px;border-radius:4px;}"
        : "QLabel{background:#1e1e2e;color:#f38ba8;padding:8px;border-radius:4px;}");
}
```

- [ ] **Step 3: Build and run**

```bash
cmake --build build --target LonelyRunnerApp && \
  ./build/src/app/LonelyRunnerApp
```

Expected: Manual Test panel is functional — enter `3, 7, 11`, click Run, see result with t = P/Q. "Animate this" button appears but opens a blank window (AnimationWidget is still a stub).

- [ ] **Step 4: Commit**

```bash
git add src/app/manualtestpanel.h src/app/manualtestpanel.cpp
git commit -m "feat: implement Manual Test panel"
```

---

## Task 13: Range Test panel

**Files:**
- Modify: `src/app/rangetestpanel.h`
- Modify: `src/app/rangetestpanel.cpp`

- [ ] **Step 1: Write rangetestpanel.h**

```cpp
#pragma once
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "data_structures.h"
#include "rangeworker.h"

class RangeTestPanel : public QWidget {
    Q_OBJECT
public:
    explicit RangeTestPanel(QWidget* parent = nullptr);

signals:
    void resultReady(std::vector<int> speeds, bool violation,
                     int numerator, int denominator);

private slots:
    void onRun();
    void onStop();
    void onProgress(int current, int total, QString status);
    void onFinished(RangeResult result);

private:
    QSpinBox*     m_startBox{};
    QSpinBox*     m_endBox{};
    QSpinBox*     m_runnersBox{};
    QSpinBox*     m_startMaxBox{};
    QCheckBox*    m_preTestBox{};
    QRadioButton* m_geoRadio{};
    QRadioButton* m_numRadio{};
    QProgressBar* m_progress{};
    QLabel*       m_statusLabel{};
    QPushButton*  m_runBtn{};
    QPushButton*  m_stopBtn{};
    QPushButton*  m_animateBtn{};
    QLabel*       m_resultLabel{};

    std::unique_ptr<RangeWorker> m_worker;
    std::vector<int> m_lastSpeeds;
    int m_lastNumerator{0}, m_lastDenominator{0};

    void setRunning(bool running);
};
```

- [ ] **Step 2: Write rangetestpanel.cpp**

```cpp
#include "rangetestpanel.h"
#include "animationwidget.h"
#include "numerical.h"
#include "util.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>

RangeTestPanel::RangeTestPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* cfgGroup  = new QGroupBox("Configuration");
    auto* cfgLayout = new QFormLayout(cfgGroup);
    m_startBox    = new QSpinBox; m_startBox->setRange(1,1000000);  m_startBox->setValue(1);
    m_endBox      = new QSpinBox; m_endBox->setRange(2,1000000);    m_endBox->setValue(100);
    m_runnersBox  = new QSpinBox; m_runnersBox->setRange(1,100);    m_runnersBox->setValue(3);
    m_startMaxBox = new QSpinBox; m_startMaxBox->setRange(1,1000000); m_startMaxBox->setValue(3);
    cfgLayout->addRow("Start:",     m_startBox);
    cfgLayout->addRow("End:",       m_endBox);
    cfgLayout->addRow("Runners:",   m_runnersBox);
    cfgLayout->addRow("Start max:", m_startMaxBox);
    layout->addWidget(cfgGroup);

    auto* optRow = new QHBoxLayout;
    m_preTestBox = new QCheckBox("Pre-test filter");
    m_geoRadio   = new QRadioButton("Geometric"); m_geoRadio->setChecked(true);
    m_numRadio   = new QRadioButton("Numerical");
    optRow->addWidget(m_preTestBox);
    optRow->addWidget(m_geoRadio);
    optRow->addWidget(m_numRadio);
    optRow->addStretch();
    layout->addLayout(optRow);

    m_progress    = new QProgressBar; m_progress->setRange(0, 100);
    m_statusLabel = new QLabel("Not running.");
    layout->addWidget(m_progress);
    layout->addWidget(m_statusLabel);

    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setStyleSheet(
        "QLabel{background:#1e1e2e;color:#cdd6f4;padding:8px;border-radius:4px;}");
    layout->addWidget(m_resultLabel, 1);

    auto* btnRow  = new QHBoxLayout;
    m_runBtn      = new QPushButton("Run");
    m_stopBtn     = new QPushButton("Stop");     m_stopBtn->setEnabled(false);
    m_animateBtn  = new QPushButton("Animate this"); m_animateBtn->setEnabled(false);
    auto* saveBtn = new QPushButton("Save JSON\xe2\x80\xa6");
    btnRow->addWidget(m_runBtn, 2);
    btnRow->addWidget(m_stopBtn, 1);
    btnRow->addWidget(m_animateBtn, 1);
    btnRow->addWidget(saveBtn, 1);
    layout->addLayout(btnRow);

    connect(m_runBtn,    &QPushButton::clicked, this, &RangeTestPanel::onRun);
    connect(m_stopBtn,   &QPushButton::clicked, this, &RangeTestPanel::onStop);

    connect(m_animateBtn, &QPushButton::clicked, this, [this]() {
        auto* w = new AnimationWidget(m_lastSpeeds, m_lastNumerator, m_lastDenominator);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_lastSpeeds.empty()) return;
        QString path = QFileDialog::getSaveFileName(
            this, "Save result", "", "JSON (*.json)");
        if (path.isEmpty()) return;
        RangeResult rr; rr.status = RangeResult::Status::Clean; rr.speeds = m_lastSpeeds;
        save_result(path.toStdString(), rr, m_lastSpeeds, "range",
                    m_lastNumerator, m_lastDenominator);
    });
}

void RangeTestPanel::onRun() {
    RangeConfig cfg;
    cfg.start_value     = m_startBox->value();
    cfg.end_value       = m_endBox->value();
    cfg.num_runners     = m_runnersBox->value();
    cfg.start_max_value = m_startMaxBox->value();
    cfg.pre_test        = m_preTestBox->isChecked();
    cfg.algorithm = m_geoRadio->isChecked() ? Algorithm::Geometric : Algorithm::Numerical;

    if (cfg.end_value <= cfg.start_value) {
        m_statusLabel->setText("End must be greater than start."); return;
    }
    if (cfg.num_runners >= cfg.end_value - cfg.start_value) {
        m_statusLabel->setText("Runners must be less than range size."); return;
    }

    m_worker = std::make_unique<RangeWorker>(cfg);
    connect(m_worker.get(), &RangeWorker::progress,  this, &RangeTestPanel::onProgress);
    connect(m_worker.get(), &RangeWorker::finished,  this, &RangeTestPanel::onFinished);
    setRunning(true);
    m_worker->start();
}

void RangeTestPanel::onStop() {
    if (m_worker) m_worker->cancel();
}

void RangeTestPanel::onProgress(int current, int total, QString status) {
    m_progress->setMaximum(total);
    m_progress->setValue(current);
    m_statusLabel->setText(status);
}

void RangeTestPanel::onFinished(RangeResult result) {
    setRunning(false);
    if (result.status == RangeResult::Status::Clean) {
        m_resultLabel->setText("\xe2\x9c\x93 No violations found.");
        m_resultLabel->setStyleSheet(
            "QLabel{background:#1e1e2e;color:#a6e3a1;padding:8px;border-radius:4px;}");
    } else if (result.status == RangeResult::Status::ViolationFound) {
        m_lastSpeeds = result.speeds;
        // Compute P/Q for animation
        auto nr = numerical_method(m_lastSpeeds);
        m_lastNumerator   = nr ? nr->a       : 0;
        m_lastDenominator = nr ? (nr->k1 + nr->k2) : 0;
        m_animateBtn->setEnabled(true);

        QString s;
        for (int i = 0; i < (int)result.speeds.size(); ++i) {
            if (i) s += ", ";
            s += QString::number(result.speeds[i]);
        }
        m_resultLabel->setText("\xe2\x9a\xa0 Possible violation: [" + s + "]");
        m_resultLabel->setStyleSheet(
            "QLabel{background:#1e1e2e;color:#f38ba8;padding:8px;border-radius:4px;}");
        emit resultReady(result.speeds, true, m_lastNumerator, m_lastDenominator);
    } else {
        m_resultLabel->setText("Cancelled.");
    }
    m_progress->setValue(m_progress->maximum());
}

void RangeTestPanel::setRunning(bool running) {
    m_runBtn->setEnabled(!running);
    m_stopBtn->setEnabled(running);
    if (!running) m_statusLabel->setText("Done.");
}
```

- [ ] **Step 3: Build and run**

```bash
cmake --build build --target LonelyRunnerApp && \
  ./build/src/app/LonelyRunnerApp
```

Expected: Range Test panel shows config fields, progress bar, Run/Stop/Animate buttons. Running range 1–20, 2 runners completes and shows "No violations found."

- [ ] **Step 4: Commit**

```bash
git add src/app/rangetestpanel.h src/app/rangetestpanel.cpp
git commit -m "feat: implement Range Test panel"
```

---

## Task 14: Verify File panel

**Files:**
- Modify: `src/app/verifyfilepanel.h`
- Modify: `src/app/verifyfilepanel.cpp`

- [ ] **Step 1: Write verifyfilepanel.h**

```cpp
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class VerifyFilePanel : public QWidget {
    Q_OBJECT
public:
    explicit VerifyFilePanel(QWidget* parent = nullptr);

private slots:
    void onChooseFile();

private:
    QLabel*      m_pathLabel{};
    QLabel*      m_resultLabel{};
};
```

- [ ] **Step 2: Write verifyfilepanel.cpp**

```cpp
#include "verifyfilepanel.h"
#include "util.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

VerifyFilePanel::VerifyFilePanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* row = new QHBoxLayout;
    auto* btn = new QPushButton("Choose result file\xe2\x80\xa6");
    m_pathLabel = new QLabel("No file selected.");
    m_pathLabel->setWordWrap(true);
    row->addWidget(btn);
    row->addWidget(m_pathLabel, 1);
    layout->addLayout(row);

    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setStyleSheet(
        "QLabel{background:#1e1e2e;color:#cdd6f4;padding:12px;border-radius:4px;}");
    layout->addWidget(m_resultLabel, 1);
    layout->addStretch();

    connect(btn, &QPushButton::clicked, this, &VerifyFilePanel::onChooseFile);
}

void VerifyFilePanel::onChooseFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open result file", "", "JSON (*.json)");
    if (path.isEmpty()) return;
    m_pathLabel->setText(path);

    try {
        bool ok = verify_result(path.toStdString());
        if (ok) {
            m_resultLabel->setText(
                "\xe2\x9c\x93  PASS \xe2\x80\x94 the saved time t is valid for these speeds.");
            m_resultLabel->setStyleSheet(
                "QLabel{background:#1e1e2e;color:#a6e3a1;padding:12px;border-radius:4px;}");
        } else {
            m_resultLabel->setText(
                "\xe2\x9c\x97  FAIL \xe2\x80\x94 the saved time t does NOT satisfy the conjecture.\n"
                "This may be a genuine violation or a corrupted file.");
            m_resultLabel->setStyleSheet(
                "QLabel{background:#1e1e2e;color:#f38ba8;padding:12px;border-radius:4px;}");
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
            QString("Could not read file:\n%1").arg(e.what()));
    }
}
```

- [ ] **Step 3: Build and run**

```bash
cmake --build build --target LonelyRunnerApp && \
  ./build/src/app/LonelyRunnerApp
```

Expected: Verify File panel shows a file picker button. Choosing a valid saved JSON shows PASS in green.

- [ ] **Step 4: Smoke test history double-click**

1. Run a manual test with speeds `3, 7, 11`
2. History entry `✓ 3,7,11` appears
3. Double-click it — Manual Test panel activates with `3,7,11` in the speed input

- [ ] **Step 5: Commit**

```bash
git add src/app/verifyfilepanel.h src/app/verifyfilepanel.cpp
git commit -m "feat: implement Verify File panel"
```

---

## Task 15: AnimationWidget

**Files:**
- Modify: `src/app/animationwidget.h`
- Modify: `src/app/animationwidget.cpp`

- [ ] **Step 1: Write animationwidget.h**

```cpp
#pragma once
#include <QWidget>
#include <QTimer>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <vector>
#include <cmath>

class AnimationCanvas : public QWidget {
    Q_OBJECT
public:
    explicit AnimationCanvas(QWidget* parent = nullptr);
    void setState(const std::vector<int>& speeds, double t, double lonely_time);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::vector<int> m_speeds;
    double m_t{0.0};
    double m_lonely_time{0.0};
};

class AnimationWidget : public QWidget {
    Q_OBJECT
public:
    AnimationWidget(std::vector<int> speeds, int lonely_num, int lonely_den,
                    QWidget* parent = nullptr);
private slots:
    void onPlayPause();
    void onReset();
    void onScrubberMoved(int value);
    void onJumpToLonely();
    void onTick();
    void onSaveImage();
private:
    std::vector<int> m_speeds;
    double  m_lonelyTime{0.0};
    double  m_t{0.0};
    double  m_speedMult{1.0};
    bool    m_playing{false};

    static constexpr int    SLIDER_STEPS    = 10000;
    static constexpr int    FRAME_MS        = 16;
    static constexpr size_t INFO_THRESHOLD  = 20;

    QTimer*          m_timer{};
    AnimationCanvas* m_canvas{};
    QSlider*         m_scrubber{};
    QPushButton*     m_playBtn{};
    QLabel*          m_timeLabel{};
    QLabel*          m_infoPanel{};
    QCheckBox*       m_showInfoBox{};

    double tMax() const;
    void   updateTimeLabel();
    void   updateInfoPanel();
    void   setScrubberFromT();
};
```

- [ ] **Step 2: Write animationwidget.cpp**

```cpp
#include "animationwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSignalBlocker>

// ── AnimationCanvas ──────────────────────────────────────────────────

AnimationCanvas::AnimationCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(300, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void AnimationCanvas::setState(const std::vector<int>& speeds, double t, double lonely_time) {
    m_speeds = speeds; m_t = t; m_lonely_time = lonely_time;
    update();
}

void AnimationCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double cx = width() / 2.0, cy = height() / 2.0;
    const double radius = std::min(width(), height()) * 0.40;
    p.fillRect(rect(), QColor("#1e1e2e"));
    if (m_speeds.empty()) return;

    const int n = static_cast<int>(m_speeds.size());
    const double threshold = 1.0 / (n + 1);

    // Lonely zone arcs (±threshold around start = angle 0)
    QColor zoneCol("#a6e3a1"); zoneCol.setAlphaF(0.18);
    auto drawZone = [&](double sign) {
        double deg = threshold * 360.0;
        QPainterPath pp;
        pp.moveTo(cx, cy);
        pp.arcTo(QRectF(cx-radius, cy-radius, 2*radius, 2*radius),
                 sign * (-deg/2.0), sign * deg);
        pp.closeSubpath();
        p.fillPath(pp, zoneCol);
    };
    drawZone(1.0); drawZone(-1.0);

    // Track
    p.setPen(QPen(QColor("#313244"), 2)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy), radius, radius);

    // Start marker
    p.setPen(Qt::NoPen); p.setBrush(QColor("#f38ba8"));
    p.drawEllipse(QPointF(cx + radius, cy), 5.0, 5.0);

    static const QColor palette[]{
        "#89b4fa","#cba6f7","#fab387","#f9e2af",
        "#a6e3a1","#94e2d5","#89dceb","#f38ba8"
    };
    const int P = static_cast<int>(sizeof(palette)/sizeof(palette[0]));

    for (int i = 0; i < n; ++i) {
        double pos = std::fmod(m_speeds[i] * m_t, 1.0);
        if (pos < 0) pos += 1.0;
        double angle = pos * 2.0 * M_PI;
        double rx = cx + radius * std::cos(-angle);
        double ry = cy + radius * std::sin(-angle);

        double dist = std::min(pos, 1.0 - pos);
        bool lonely = dist >= threshold;
        QColor col = palette[i % P];

        if (lonely) {
            QColor ring = col; ring.setAlphaF(0.35);
            p.setPen(QPen(ring, 2)); p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(rx, ry), 11.0, 11.0);
        }
        p.setPen(Qt::NoPen); p.setBrush(col);
        p.drawEllipse(QPointF(rx, ry), 7.0, 7.0);
    }
}

// ── AnimationWidget ──────────────────────────────────────────────────

static const char* SPEED_LABELS[]{"0.5\xc3\x97","1\xc3\x97","2\xc3\x97","5\xc3\x97"};
static const double SPEED_VALUES[]{0.5, 1.0, 2.0, 5.0};

AnimationWidget::AnimationWidget(std::vector<int> speeds, int lonely_num, int lonely_den,
                                 QWidget* parent)
    : QWidget(parent, Qt::Window),
      m_speeds(std::move(speeds)),
      m_lonelyTime(lonely_den > 0 ? double(lonely_num) / lonely_den : 0.0)
{
    setWindowTitle("Animation — Lonely Runner");
    resize(720, 540);

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0,0,0,0); vbox->setSpacing(0);

    // Canvas + info panel side by side
    m_canvas    = new AnimationCanvas(this);
    m_infoPanel = new QLabel;
    m_infoPanel->setFixedWidth(175);
    m_infoPanel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_infoPanel->setStyleSheet(
        "QLabel{background:#181825;color:#cdd6f4;padding:10px;font-family:monospace;}");

    auto* canvasRow = new QHBoxLayout;
    canvasRow->addWidget(m_canvas, 1);
    canvasRow->addWidget(m_infoPanel);
    vbox->addLayout(canvasRow, 1);

    // Warning banner for high runner count
    if (m_speeds.size() > INFO_THRESHOLD) {
        auto* warn = new QLabel(
            "\xe2\x9a\xa0 High runner count \xe2\x80\x94 detail hidden for clarity.");
        warn->setStyleSheet("background:#f9e2af;color:#1e1e2e;padding:4px;");
        m_showInfoBox = new QCheckBox("Show info anyway");
        auto* warnRow = new QHBoxLayout;
        warnRow->addWidget(warn); warnRow->addWidget(m_showInfoBox);
        vbox->addLayout(warnRow);
        m_infoPanel->setVisible(false);
        connect(m_showInfoBox, &QCheckBox::toggled,
                m_infoPanel,   &QLabel::setVisible);
    }

    // Scrubber
    m_scrubber = new QSlider(Qt::Horizontal);
    m_scrubber->setRange(0, SLIDER_STEPS);
    vbox->addWidget(m_scrubber);

    // Control bar
    auto* bar       = new QWidget; bar->setStyleSheet("background:#181825;");
    auto* barLayout = new QHBoxLayout(bar);
    barLayout->setContentsMargins(8,6,8,6);

    m_playBtn        = new QPushButton("\xe2\x96\xb6 Play");
    auto* resetBtn   = new QPushButton("\xe2\x86\xa9 Reset");
    auto* jumpBtn    = new QPushButton("Jump to t");
    jumpBtn->setEnabled(m_lonelyTime > 0.0);

    barLayout->addWidget(m_playBtn);
    barLayout->addWidget(resetBtn);
    barLayout->addWidget(jumpBtn);
    barLayout->addSpacing(12);
    barLayout->addWidget(new QLabel("Speed:"));

    // Speed multiplier buttons
    QWidget* speedGroup = new QWidget;
    auto* speedRow = new QHBoxLayout(speedGroup);
    speedRow->setContentsMargins(0,0,0,0); speedRow->setSpacing(2);
    for (int i = 0; i < 4; ++i) {
        auto* btn = new QPushButton(SPEED_LABELS[i]);
        btn->setCheckable(true); btn->setChecked(i == 1);
        double sv = SPEED_VALUES[i];
        connect(btn, &QPushButton::clicked, this, [this, sv, i, speedGroup]() {
            m_speedMult = sv;
            for (int j = 0; j < speedGroup->layout()->count(); ++j) {
                auto* b = qobject_cast<QPushButton*>(
                    speedGroup->layout()->itemAt(j)->widget());
                if (b) b->setChecked(j == i);
            }
        });
        speedRow->addWidget(btn);
    }
    barLayout->addWidget(speedGroup);
    barLayout->addStretch();

    m_timeLabel = new QLabel("t = 0.0000");
    m_timeLabel->setStyleSheet("color:#89b4fa;");
    barLayout->addWidget(m_timeLabel);

    auto* saveBtn = new QPushButton("\xe2\xac\x87 PNG");
    barLayout->addWidget(saveBtn);
    vbox->addWidget(bar);

    // Timer
    m_timer = new QTimer(this);
    m_timer->setInterval(FRAME_MS);

    connect(m_timer,    &QTimer::timeout,         this, &AnimationWidget::onTick);
    connect(m_playBtn,  &QPushButton::clicked,    this, &AnimationWidget::onPlayPause);
    connect(resetBtn,   &QPushButton::clicked,    this, &AnimationWidget::onReset);
    connect(jumpBtn,    &QPushButton::clicked,    this, &AnimationWidget::onJumpToLonely);
    connect(m_scrubber, &QSlider::valueChanged,   this, &AnimationWidget::onScrubberMoved);
    connect(saveBtn,    &QPushButton::clicked,    this, &AnimationWidget::onSaveImage);

    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel();
    updateInfoPanel();
}

double AnimationWidget::tMax() const {
    return std::max(1.0, m_lonelyTime * 2.5);
}

void AnimationWidget::onTick() {
    m_t += m_speedMult * FRAME_MS / 1000.0;
    if (m_t > tMax()) m_t = 0.0;
    setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}

void AnimationWidget::onPlayPause() {
    m_playing = !m_playing;
    m_playBtn->setText(m_playing ? "\xe2\x8f\xb8 Pause" : "\xe2\x96\xb6 Play");
    m_playing ? m_timer->start() : m_timer->stop();
}

void AnimationWidget::onReset() {
    m_timer->stop(); m_playing = false;
    m_playBtn->setText("\xe2\x96\xb6 Play");
    m_t = 0.0; setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}

void AnimationWidget::onJumpToLonely() {
    m_t = m_lonelyTime; setScrubberFromT();
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}

void AnimationWidget::onScrubberMoved(int value) {
    if (m_timer->isActive()) return;
    m_t = tMax() * double(value) / SLIDER_STEPS;
    m_canvas->setState(m_speeds, m_t, m_lonelyTime);
    updateTimeLabel(); updateInfoPanel();
}

void AnimationWidget::setScrubberFromT() {
    int v = static_cast<int>(m_t / tMax() * SLIDER_STEPS);
    QSignalBlocker blocker(m_scrubber);
    m_scrubber->setValue(std::clamp(v, 0, SLIDER_STEPS));
}

void AnimationWidget::onSaveImage() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save image", "animation.png", "PNG (*.png)");
    if (!path.isEmpty()) m_canvas->grab().save(path);
}

void AnimationWidget::updateTimeLabel() {
    m_timeLabel->setText(QString("t = %1").arg(m_t, 0, 'f', 4));
}

void AnimationWidget::updateInfoPanel() {
    if (!m_infoPanel->isVisible()) return;

    const int    n         = static_cast<int>(m_speeds.size());
    const double threshold = 1.0 / (n + 1);

    QString text;
    text += QString("<b>t = %1</b><br>").arg(m_t, 0, 'f', 5);
    if (m_lonelyTime > 0.0)
        text += QString("Lonely t \xe2\x89\x88 %1<br>").arg(m_lonelyTime, 0, 'f', 5);
    text += QString("Threshold: 1/%1 = %2<br><br>")
            .arg(n + 1).arg(threshold, 0, 'f', 4);
    text += "<b>Distances:</b><br>";
    for (int i = 0; i < n; ++i) {
        double pos  = std::fmod(m_speeds[i] * m_t, 1.0);
        if (pos < 0) pos += 1.0;
        double dist = std::min(pos, 1.0 - pos);
        bool lonely = dist >= threshold;
        text += QString("v=%1 &nbsp;%2 %3<br>")
                .arg(m_speeds[i])
                .arg(dist, 0, 'f', 4)
                .arg(lonely ? "\xe2\x9c\x93" : "\xe2\x9c\x97");
    }
    m_infoPanel->setText(text);
}
```

- [ ] **Step 3: Build and run**

```bash
cmake --build build --target LonelyRunnerApp && \
  ./build/src/app/LonelyRunnerApp
```

Expected:
1. Manual Test → speeds `3,7,11` → Run → "Animate this" → floating window opens
2. Circular track with three coloured dots
3. Green zone arc around start
4. Play/Pause, speed buttons, jump-to-t, scrubber all functional
5. Info panel shows per-runner distances and threshold
6. For >20 runners: warning banner and collapsed info panel with toggle

- [ ] **Step 4: Run ctest to confirm all automated tests still pass**

```bash
ctest --test-dir build --output-on-failure
```

Expected: `1/1 Test #1: unit_tests ... Passed`

- [ ] **Step 5: Commit**

```bash
git add src/app/animationwidget.h src/app/animationwidget.cpp
git commit -m "feat: implement AnimationWidget with canvas, controls, info panel, threshold"
```

---

## Self-Review

**Spec coverage:**

| Requirement | Task |
|---|---|
| Bug fix #1 (Geo missing return length<1) | Task 5 |
| Bug fix #2 (Geo no return at FINAL) | Task 5 |
| Bug fix #3 (Geo UB at end of function) | Task 5 |
| Bug fix #4 (rounds signed/unsigned) | Task 2 + 5 |
| Bug fix #5 (isValid break outside if) | Task 3 |
| Bug fix #6 (ZZ_mod O(n) loop) | Task 3 |
| Bug fix #7 (unsigned sentinel -1) | Task 4 |
| Bug fix #8 (candidate_k never updated) | Task 4 |
| Bug fix #9 (delete tm) | Removed with GTK in Task 1 |
| Bug fix #10 (g_free vs delete) | Removed with GTK in Task 1 |
| Bug fix #11 (VLAs) | Task 9 (std::vector) |
| Bug fix #12 (off-by-one range loop) | Task 9 |
| Bug fix #13 (checkForSolution duplicate) | Task 3 (single impl) |
| Advanced_Geometric_method removal | Task 5 (not ported) |
| CMake build system | Task 1 |
| C++20, liblonelyrunner no Qt | Tasks 1–9 |
| NTL + GMP kept | Tasks 3–5 |
| nlohmann/json replaces libjson-c | Task 7 |
| Parallel range test (QtConcurrent) | Task 10 |
| Qt6 sidebar nav MainWindow | Task 11 |
| Manual Test panel | Task 12 |
| Range Test panel + progress + Stop | Task 13 |
| Verify File panel | Task 14 |
| History sidebar + double-click restore | Task 11 + 14 |
| Animation floating window | Task 15 |
| Circular track + runners + zone | Task 15 |
| Info panel + per-runner distances | Task 15 |
| Threshold (>20 runners) + toggle | Task 15 |
| Jump-to-lonely-moment button | Task 15 |
| Time scrubber (QSlider) | Task 15 |
| Save PNG | Task 15 |
| Save/verify JSON round-trip | Tasks 7–8 |
| Test binary + ctest | Tasks 3–9 |
| --verify CLI | Task 9 |

**Placeholder scan:** None found.

**Type consistency:** `RangeResult`, `RangeConfig`, `NumResult`, `GeoResult` defined in Task 2, used consistently through Task 15. `range_test_sequential` defined in Task 9, called in Task 10. `AnimationWidget(speeds, num, den)` constructor defined and called with identical signatures in Tasks 12, 13, 15.
