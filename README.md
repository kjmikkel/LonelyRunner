# Lonely Runner Conjecture Verifier

A tool for experimentally verifying the Lonely Runner Conjecture, originally written in 2010 as part of a Master's project at the University of Copenhagen, and modernized in 2026 into a C++20 Qt6 desktop application.

## The Lonely Runner Conjecture

Imagine *n* runners on a circular track of unit length, each running at a constant, pairwise distinct speed. The conjecture states: for every runner, there exists a time *t* at which that runner is at least 1/(*n* + 1) units away from the common start point.

The conjecture has been proved for up to *n* = 7 runners. This tool lets you verify it experimentally for arbitrary speed sets.

## Features

- **Manual Test** — enter a set of runner speeds, verify the conjecture, and see the lonely time
- **Range Test** — parallel sweep over all speed combinations up to a given maximum; reports any violation found
- **Verify File** — load a JSON file of speed sets and batch-verify them all
- **Animation window** — live visualisation of runners on the track:
  - Colour-coded runner dots with a matching legend in the info panel
  - Speed labels radiating outward from each dot (shown for ≤ 15 runners)
  - Logarithmic speed slider (0.06× – 2×) for fine-grained control
  - "Jump to lonely time" button to jump directly to the verified moment
- **Light / Dark / System theme** — Catppuccin Latte (light) and Catppuccin Mocha (dark) palettes, switchable in the Options panel; preference is saved across sessions

## Build

Requires WSL2 or a native Linux desktop (Ubuntu/Debian, Fedora, or Arch).

**Quickstart — installs dependencies and builds in one step:**

```bash
./setup.sh
```

`setup.sh` detects your distribution, installs the required packages via the
native package manager (`apt` / `dnf` / `pacman`), and then runs the cmake
build automatically.

**Manual steps** (if you prefer to install dependencies yourself):

```bash
# Ubuntu / Debian
sudo apt install libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build

# Fedora
sudo dnf install ntl-devel gmp-devel qt6-qtbase-devel cmake ninja-build

# Arch
sudo pacman -S ntl gmp qt6-base cmake ninja

# Then build and test
cmake -B build -G Ninja && cmake --build build
ctest --test-dir build --output-on-failure   # should report 213/213 passed
```

**Run:**

```bash
LIBGL_ALWAYS_SOFTWARE=1 ./build/src/app/LonelyRunnerApp
```

> **WSL2 note:** The `LIBGL_ALWAYS_SOFTWARE=1` prefix silences benign Mesa/EGL warnings
> about a missing GPU driver. The application uses CPU-based Qt painting and works
> correctly without hardware OpenGL.

## Architecture

Three CMake targets:

| Target | Description | Location |
|--------|-------------|----------|
| `lonelyrunner` | Static core library — numerical, geometric, and prime algorithms; no Qt dependency | `src/core/` |
| `LonelyRunnerApp` | Qt6 GUI application | `src/app/` |
| `LonelyRunnerTests` | Unit test binary (links only `lonelyrunner`) | `src/test/` |

The original 2010 GTK+2 / C++03 source is preserved in `src/C++/` but is not compiled by the current build system.

## Version history and the report

The academic report in `report/` documents the algorithms as implemented in the **original 2010 codebase**. That code (GTK+2, C++03) is preserved at tag `v1.0.0` and reflects exactly what was submitted for grading. Tag `v1.1` is a snapshot of the same code taken just before any LLM involvement.

The current `master` branch (tag `v2.0`) is a full reimagining of the application and is **no longer in sync** with the report. If you want the code the report describes:

```bash
git checkout v1.0.0
```

Original test data (JSON format) and the graphs used in the report remain in `data/` and `report/data/` respectively.

## Algorithm notes

The application implements two independent methods for finding lonely times:

**Geometric method** (`src/core/geometric.cpp`) — the complete verifier. It models each runner's
lonely zone (distance ≥ 1/(n+1) from the start) as a series of time intervals and sweeps a
priority-queue timeline. Every runner is in the lonely zone simultaneously at the moment the
overlap count reaches n. The method is complete within its search window [0, 2(n+1)] and is
used for all range-test violation detection.

**Numerical method** (`src/core/numerical.cpp`) — a targeted lonely-time finder. It searches
denominators of the form `speeds[i] + speeds[j]` (derived from two runners being symmetrically
placed at the zone boundary). This search is not exhaustive — it may miss lonely times with
other denominators — so a `nullopt` from this method does **not** prove the conjecture is
violated. The numerical method is used in the Manual Test panel to show a specific lonely time.
The range test always uses the geometric method for the yes/no violation determination.

**Prime Modular method** (`src/core/prime_modular.cpp`) — searches lonely times of the form
t = a/((n+1)·p) for small primes p. This denominator family is central to the 2025–2026
proofs by Matthieu Rosenfeld (8 runners, arXiv:2509.14111) and Tanupat Trakulthongchai
(9–10 runners, arXiv:2511.22427): any counterexample's speed-product must be divisible by
primes for which no tuple covers the range with this denominator. Here the search runs in the
forward direction — given a specific speed set, it checks each t = a/((n+1)·p) for primes
up to 200. Like the numerical method, it is not exhaustive, but it covers a complementary set
of denominators and runs in pure integer arithmetic (no NTL required).

## License

Released under the GPLv3 — see `src/LICENSE`.
