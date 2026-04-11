# Lonely Runner Verifier — Modernization

## Project goal
Modernize a GTK+2 C++03 application (2010) into a clean C++20 Linux app with Qt6 GUI,
parallel range test, and runner animation. **Linux-only target** (WSL2 on Windows 11).

## Build (run inside WSL2)
```bash
# First-time setup (installs deps + builds):
./setup.sh

# Subsequent builds:
cmake --build build

# Tests:
ctest --test-dir build --output-on-failure

# Run:
LIBGL_ALWAYS_SOFTWARE=1 ./build/src/app/LonelyRunnerApp
```

`LIBGL_ALWAYS_SOFTWARE=1` silences benign Mesa/EGL warnings about the missing GPU driver
in WSL2. The app uses CPU-based Qt painting and works correctly without it.

`setup.sh` detects the distro and installs the right packages via `apt` / `dnf` / `pacman`.
Manual dependency install (Ubuntu/WSL2): `sudo apt install libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build`

## Current status (2026-04-11)
All targets build and link cleanly. All 170 unit tests pass.

Core algorithm fixes applied (TDD red/green pass):
- **Bug 1 fixed** (`geometric.cpp`): `CompareEntries` tiebreaker now enforces strict total order
  (STARTs before ENDs before FINAL at equal time; lower runner_number first within type).
  Previously violated strict weak ordering when a START and END from different runners coincided,
  causing the priority queue to silently miss solutions.
- **Bug 2 fixed** (`range_test.cpp`): Range test always uses the geometric method for violation
  detection. Previously, a `nullopt` from `numerical_method` (which only searches pair-sum
  denominators — an incomplete search) was misread as a violation, producing false positives.
- **Overflow fix** (`numerical.cpp`): denominator `k` is now computed as `NTL::ZZ` before the
  inner loop, eliminating potential `int` overflow for large speed values.
- All three core files (`geometric.cpp`, `numerical.cpp`, `range_test.cpp`) have been rewritten
  for clarity: precomputed `NTL::ZZ` speed vectors, `make_entry` helper, renamed variables,
  removed `using namespace NTL`, added mathematical comments explaining the time-unit model.

- `lonelyrunner` static lib — complete (numerical, geometric, prime algorithms; NTL arbitrary precision)
- `LonelyRunnerTests` — 170/170 tests pass
- `LonelyRunnerApp` — full Qt6 GUI, complete:
  - Manual Test, Range Test (parallel `RangeWorker`), Verify File panels
  - Animation window with colour-coded runner dots, speed legend, log speed slider (0.06×–2×)
  - Light / Dark / System theme support (Catppuccin Latte / Mocha) with persistent QSettings
  - Global stylesheet (rounded buttons, group boxes, input fields, list widgets)
  - Title-bar dark-mode hint via `xprop` (X11/WSL2)

The MOC fix was `set_target_properties(LonelyRunnerApp PROPERTIES AUTOMOC ON)`
in `src/app/CMakeLists.txt` — `qt6_add_executable` does not enable it automatically.

## Architecture
Three CMake targets:
- `lonelyrunner` — static lib, zero Qt dependency (`src/core/`)
- `LonelyRunnerApp` — Qt6 GUI (`src/app/`)
- `LonelyRunnerTests` — test binary, links only `lonelyrunner` (`src/test/`)

The original GTK code is archived in `src/C++/` — nothing there is compiled.

## Documentation maintenance

Keep these files up to date as the project evolves:

- **CLAUDE.md**: Update "Current status" whenever a significant change lands — new features,
  bug fixes, test count changes, architectural decisions, or anything a future Claude session
  needs to know before starting work.
- **README.md**: Update whenever user-visible behaviour changes — new features, changed build
  steps, dependency changes, algorithm behaviour changes, or corrections to the version history.

When the test count changes, update the count in both files.

## Style
C++20, `std::optional` / `std::span` / value semantics throughout. No raw `new`/`delete`
in the new code. NTL (`#include <NTL/ZZ.h>`) is used for arbitrary-precision arithmetic
in the numerical and geometric algorithms.
