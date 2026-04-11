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
All targets build and link cleanly. All 33 unit tests pass.

- `lonelyrunner` static lib — complete (numerical, geometric, prime algorithms; NTL arbitrary precision)
- `LonelyRunnerTests` — 33/33 tests pass
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

## Style
C++20, `std::optional` / `std::span` / value semantics throughout. No raw `new`/`delete`
in the new code. NTL (`#include <NTL/ZZ.h>`) is used for arbitrary-precision arithmetic
in the numerical and geometric algorithms.
