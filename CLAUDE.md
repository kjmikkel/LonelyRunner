# Lonely Runner Verifier — Modernization

## Project goal
Modernize a GTK+2 C++03 application (2010) into a clean C++20 Linux app with Qt6 GUI,
parallel range test, and runner animation. **Linux-only target** (WSL2 on Windows 11).

## Key documents
- **Design spec:** `docs/superpowers/specs/2026-04-10-lonely-runner-modernization-design.md`
- **Implementation plan:** `docs/superpowers/plans/2026-04-10-lonely-runner-modernization.md`
  Read the plan before doing any implementation work. Tasks have checkboxes — mark them as
  you go.

## Build (run inside WSL2)
```bash
cmake -B build -G Ninja && cmake --build build
ctest --test-dir build --output-on-failure
./build/src/app/LonelyRunnerApp   # requires a display; use WSL2 with WSLg or X11
```

Dependencies (already installed in Ubuntu-22.04 WSL2):
`libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build`

## Current status (2026-04-10)
All targets build and link cleanly. All 33 unit tests pass.

- `lonelyrunner` static lib — complete
- `LonelyRunnerTests` — 33/33 tests pass
- `LonelyRunnerApp` — links cleanly; full Qt6 GUI with Manual Test, Range Test,
  Verify File panels, runner animation window, and parallel `RangeWorker`

The MOC fix was `set_target_properties(LonelyRunnerApp PROPERTIES AUTOMOC ON)`
in `src/app/CMakeLists.txt` — `qt6_add_executable` does not enable it automatically.

**Next step:** Manual UI smoke-test (requires WSLg / X11 display):
```bash
./build/src/app/LonelyRunnerApp
```

## Architecture
Three CMake targets:
- `lonelyrunner` — static lib, zero Qt dependency (`src/core/`)
- `LonelyRunnerApp` — Qt6 GUI (`src/app/`)
- `LonelyRunnerTests` — test binary, links only `lonelyrunner` (`src/test/`)

The original GTK code is archived in `src/C++/` — nothing there is compiled.

## Style
C++20, `std::optional` / `std::span` / value semantics throughout. No raw `new`/`delete`
in the new code. NTL (`#include <NTL/ZZ.h>`) is used for arbitrary-precision arithmetic
in the numerical and geometric algorithms.
