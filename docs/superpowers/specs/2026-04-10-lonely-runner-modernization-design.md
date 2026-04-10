# Lonely Runner Verifier — Modernization Design

**Date:** 2026-04-10  
**Status:** Approved

---

## Overview

Modernize the Lonely Runner Conjecture Verifier — a C++ GTK+2 application from 2010 — into a clean, maintainable Linux application with a Qt6 GUI, parallel computation, and an animated visualization of the conjecture.

**Goals (in order of priority):**
1. Fix all identified bugs
2. Modernize the C++ codebase (C++20)
3. Linux-only target (including WSL2 on Windows 11)
4. Parallel range test computation
5. Improved Qt6 GUI
6. Runner animation

---

## Section 1: Architecture & Project Structure

Three CMake targets:

```
LonelyRunner/
├── CMakeLists.txt
├── src/
│   ├── core/                       # Static lib: liblonelyrunner
│   │   ├── CMakeLists.txt
│   │   ├── data_structures.h
│   │   ├── geometric.h/.cpp
│   │   ├── numerical.h/.cpp
│   │   ├── prime.h/.cpp
│   │   ├── util.h/.cpp
│   │   └── range_test.h/.cpp
│   ├── app/                        # Qt6 GUI binary
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   ├── mainwindow.h/.cpp
│   │   ├── rangeworker.h/.cpp
│   │   └── animationwidget.h/.cpp
│   └── test/                       # Test binary
│       ├── CMakeLists.txt
│       └── tests.cpp
└── data/                           # JSON test data
```

**Key boundary:** `liblonelyrunner` has zero Qt dependency. It takes plain C++ inputs and returns plain C++ results. The `app` target owns all Qt code. The parallel range test in `range_test.cpp` uses `std::async` + `std::atomic` for cancellation with no knowledge of the GUI. `rangeworker.cpp` bridges it to Qt signals/slots.

---

## Section 2: Build System

- **Build system:** CMake 3.20+, Ninja generator
- **C++ standard:** C++20 across all targets
- **Dependencies via apt:**
  ```bash
  apt install libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build
  ```
- **nlohmann/json:** Single header bundled under `third_party/` — no package needed
- **`liblonelyrunner`:** Static library, `-O2`, links `ntl gmp m`
- **`LonelyRunnerApp`:** Links `liblonelyrunner Qt6::Widgets Qt6::Concurrent`
- **`LonelyRunnerTests`:** Links `liblonelyrunner` only, registered with `ctest`

Single build invocation:
```bash
cmake -B build -G Ninja && cmake --build build
```

---

## Section 3: Core Computation — Bug Fixes & Modernization

### Bugs Fixed

**Geometric.cpp**

1. `Geometric_method` (length < 1 path): `no_result` created but never returned — falls through to queue allocation. **Fix:** add `return no_result`.
2. `Geometric_method` (end of function): queue can empty without hitting `FINAL` — falls off end with no return (UB). **Fix:** return null-result after the loop.
3. `Advanced_Geometric_method`: body almost entirely commented out, no return statement (UB). **Fix:** remove entirely.
4. `make_start_point`: passes `rounds = -1` into `unsigned int rounds` — signed/unsigned wrap-around. **Fix:** change `rounds` field to `int` in `event_point`.

**Numerical.cpp**

5. `isValidInternal`: `break` sits outside `if (!valid)` block — always breaks after checking the first runner, making multi-runner validation silently wrong. **Fix:** wrap `cout` and `break` inside the `if`.
6. `ZZ_mod`: O(n) subtraction loop. **Fix:** use NTL's `%` operator directly.
7. `unsigned int candidate_k1 = -1` / `candidate_k2 = -1`: unsigned wrap-around as sentinel alongside a `no_candidate` bool that already tracks this. **Fix:** remove the sentinel values; rely solely on `no_candidate`.
8. `candidate_k` initialized to `to_ZZ(-1)` and never updated in the loop — `candidate_comparison` silently compares against the wrong value for all subsequent candidates. **Fix:** update `candidate_k` when a new best candidate is stored.

**main.c**

9. `delete tm`: `tm` declared but never assigned — deleting uninitialised pointer (UB). **Fix:** remove it.
10. `delete filename` on GTK-allocated strings: should be `g_free`. Moot after GTK removal; Qt uses `QString`, no manual free needed.
11. VLAs `int test_array[num_runners]` and `int real_number_array[max_number]`: non-standard in C++. **Fix:** `std::vector<int>`.
12. Off-by-one in `range_test` loop: `index <= max_number` but array has size `max_number`. **Fix:** `index < max_number`.

**Structural**

13. `checkForSolution` duplicated identically in `main.c` and `Numerical.cpp`. **Fix:** single implementation in `core/numerical.cpp`, exposed in the header.

### Modernised API

```cpp
// core/data_structures.h
struct GeoResult {
    bool found;
    event_point point;   // meaningful only if found == true
};

struct NumResult {
    bool found;
    int k1, k2, a;       // meaningful only if found == true
};

// core/geometric.h
std::optional<GeoResult> geometric_method(std::span<const int> speeds);

// core/numerical.h
std::optional<NumResult> numerical_method(
    std::span<const int> speeds,
    bool find_maximum = false);

bool is_valid(const NumResult&, std::span<const int> speeds);
bool is_valid(const GeoResult&, std::span<const int> speeds);
bool check_for_solution(std::span<const int> speeds);
```

`std::optional` replaces all raw `new`/`delete` result pointers. No heap allocation, no ownership ambiguity.

---

## Section 4: Parallel Range Test

### Approach

The outer loop in `range_test` iterates over the "top value" — the largest speed in each combination being tested. Each top value is fully independent.

**Mechanism:** `QtConcurrent::map` over the range of top-value indices.
- Automatic thread pool sized to hardware concurrency
- `QFuture<void>` + `QFutureWatcher` for progress signals to GUI
- `future.cancel()` wired to a Stop button

**Cancellation:** When a violation candidate is found, the discovering thread sets `std::atomic<bool> cancelled`. All worker threads check this flag at the top of each recursive call and exit early. The watcher signals the GUI with the offending speed set.

**Progress:** Each completed top-value iteration increments `std::atomic<int> completed`. `QFutureWatcher::progressValueChanged` drives a progress bar and status label.

**Thread safety:** Each worker operates on its own local copy of the speed array. No shared mutable state between workers. Result emission is via Qt signal to main thread only.

### API

```cpp
// core/range_test.h
struct RangeConfig {
    int start_value;
    int end_value;
    int num_runners;
    int start_max_value;
    bool pre_test;
    enum class Algorithm { Geometric, Numerical } algorithm;
};

struct RangeResult {
    enum class Status { Clean, ViolationFound, Cancelled };
    Status status;
    std::vector<int> speeds;              // populated if ViolationFound
    std::chrono::microseconds elapsed;
};

RangeResult range_test(
    const RangeConfig& config,
    std::atomic<bool>& cancel_flag,
    std::function<void(int current, int total)> progress_cb);
```

`rangeworker.cpp` wraps this in a `QThread`, translating progress callbacks and the final `RangeResult` into Qt signals.

### Save / Verify

Every violation candidate (and any result that fails `is_valid`) can be saved to JSON for later reproduction and independent verification.

**Save format:**
```json
{
  "version": 1,
  "type": "violation_candidate",
  "speeds": [3, 7, 11, 13],
  "num_runners": 4,
  "algorithm": "numerical",
  "result_found": false,
  "time": { "numerator": 0, "denominator": 0 },
  "timestamp": "2026-04-10T12:00:00Z"
}
```
When a solution is found but fails `is_valid`, the fraction P/Q is included.

**Verification paths:**
- GUI: "Verify from file" action loads JSON and runs `verify_result()`, displaying pass/fail
- CLI: `LonelyRunnerTests --verify violation_candidate.json` for headless verification

**Core API additions:**
```cpp
// core/util.h
void save_result(const std::filesystem::path&, const RangeResult&);
RangeResult load_result(const std::filesystem::path&);
bool verify_result(const RangeResult&);
```

---

## Section 5: Qt6 GUI

### Main Window (`QMainWindow`)

Sidebar navigation (left) with four entries: Manual Test, Range Test, Animation, Verify File. History list at the bottom of the sidebar shows last N results (✓ green / ⚠ red), clickable to reload that configuration. Central area is a `QStackedWidget` switching content per sidebar selection.

### Manual Test Panel

- Free-text speed input (comma-separated integers) + JSON import button
- Checkboxes: pre-test filter, find maximum t
- Radio buttons: Geometric / Numerical algorithm
- Result box: solution found/not found, time fraction P/Q, verification status, elapsed time
- "Animate this" button appears once a result exists

### Range Test Panel

- Integer fields: start, end, num_runners, start_max_value
- Pre-test checkbox, algorithm radio buttons
- Progress bar + status label (current combination, estimated time remaining)
- Stop button via `QFuture::cancel()`
- On completion: result box with Save JSON button and, if a solution was found, "Animate this"

### Verify File Panel

- File picker loading a saved result JSON
- Runs `verify_result()` from core library
- Displays pass/fail with full detail (speeds, P/Q, which runners failed if invalid)

### Animation Window (`QWidget`, floating)

A separate top-level `QWidget` (not a child of `MainWindow`). Opens via "Animate this" or the Animation sidebar entry.

**Canvas (QPainter):**
- Circular track
- Coloured runner dots
- 1/(n+1) arc zone (green fill)
- Lonely runner highlighted with pulsing ring
- Dashed line to nearest neighbour

**Info panel (right side):**
- Time as decimal and exact fraction P/Q
- Per-runner distances from start
- Threshold value 1/(n+1)
- Lonely moment indicator

**Controls:**
- Play / Pause, Reset
- Speed multipliers: 0.5×, 1×, 2×, 5×
- **Jump to t** button — skips instantly to the verified lonely moment P/Q
- **Time scrubber** (QSlider) — drag to any point in time; range mapped to LCM of all runner periods
- Save PNG (`QPixmap::save`) and Save JSON buttons

**Threshold behaviour:**
- ≤ 20 runners: full info panel visible
- > 20 runners: info panel collapses automatically; yellow warning banner: *"High runner count — detail hidden for clarity. Enable manually."* with a toggle button

---

## Section 6: Test Binary

No external test framework. Lightweight assertion-based harness registered with `ctest`.

**Bug regression tests** — one test per bug fixed:
- `isValidInternal` single-runner short-circuit (break outside if)
- `Geometric_method` empty-list return path
- `Geometric_method` queue-exhaustion return path
- `Numerical_method` candidate_k never-updated path
- Off-by-one in range loop

**Known-good result tests:**
- Small hand-verifiable sets (`{1, 2}`, `{2, 3, 4}`)
- Sets from existing `data/` JSON files — load, run both algorithms, confirm results match

**Validation tests:**
- `is_valid` accepts valid results
- `is_valid` rejects deliberately corrupted P/Q values

**Save/load round-trip:**
- Save `RangeResult` to temp file, reload, verify deserialized data matches, `verify_result()` passes

**Parallel smoke test:**
- Range (1–20, 3 runners) via parallel path produces same result as sequential reference

**Running:**
```bash
cmake --build build --target LonelyRunnerTests
ctest --test-dir build --output-on-failure
# Headless verification:
./build/LonelyRunnerTests --verify path/to/violation.json
```
