#!/usr/bin/env bash
# setup.sh — install dependencies and build the Lonely Runner Verifier
# Supports Ubuntu/Debian, Fedora/RHEL, and Arch Linux.

set -e

# ---------------------------------------------------------------------------
# Detect distro
# ---------------------------------------------------------------------------

if [ ! -f /etc/os-release ]; then
    echo "ERROR: Cannot detect Linux distribution (no /etc/os-release)."
    echo "Please install the following manually, then run:"
    echo "  cmake -B build -G Ninja && cmake --build build"
    echo ""
    echo "Required libraries: NTL, GMP, Qt6 base"
    echo "Required tools:     cmake, ninja"
    exit 1
fi

. /etc/os-release

# ---------------------------------------------------------------------------
# Install dependencies
# ---------------------------------------------------------------------------

install_apt() {
    echo "Detected: $PRETTY_NAME (apt)"
    sudo apt-get update -qq
    sudo apt-get install -y \
        libntl-dev \
        libgmp-dev \
        qt6-base-dev \
        qt6-base-dev-tools \
        cmake \
        ninja-build
}

install_dnf() {
    echo "Detected: $PRETTY_NAME (dnf)"
    sudo dnf install -y \
        ntl-devel \
        gmp-devel \
        qt6-qtbase-devel \
        cmake \
        ninja-build
}

install_pacman() {
    echo "Detected: $PRETTY_NAME (pacman)"
    sudo pacman -Sy --needed \
        ntl \
        gmp \
        qt6-base \
        cmake \
        ninja
}

case "$ID" in
    ubuntu|debian|linuxmint|pop)
        install_apt
        ;;
    fedora)
        install_dnf
        ;;
    rhel|centos|almalinux|rocky)
        # EPEL may be needed for NTL on RHEL-based systems
        sudo dnf install -y epel-release 2>/dev/null || true
        install_dnf
        ;;
    arch|manjaro|endeavouros|garuda)
        install_pacman
        ;;
    *)
        # Try ID_LIKE as fallback (e.g. ID=linuxmint ID_LIKE=ubuntu)
        case "$ID_LIKE" in
            *debian*|*ubuntu*)
                install_apt
                ;;
            *fedora*|*rhel*)
                install_dnf
                ;;
            *arch*)
                install_pacman
                ;;
            *)
                echo "ERROR: Unsupported distribution: $PRETTY_NAME"
                echo ""
                echo "Please install the following manually:"
                echo "  Ubuntu/Debian:  sudo apt install libntl-dev libgmp-dev qt6-base-dev qt6-base-dev-tools cmake ninja-build"
                echo "  Fedora:         sudo dnf install ntl-devel gmp-devel qt6-qtbase-devel cmake ninja-build"
                echo "  Arch:           sudo pacman -S ntl gmp qt6-base cmake ninja"
                echo ""
                echo "Then build with:"
                echo "  cmake -B build -G Ninja && cmake --build build"
                exit 1
                ;;
        esac
        ;;
esac

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------

echo ""
echo "Dependencies installed. Building..."
echo ""

cmake -B build -G Ninja
cmake --build build

echo ""
echo "Build complete. Run tests with:"
echo "  ctest --test-dir build --output-on-failure"
echo ""
echo "Launch the app with:"
echo "  LIBGL_ALWAYS_SOFTWARE=1 ./build/src/app/LonelyRunnerApp"
echo ""
echo "  (LIBGL_ALWAYS_SOFTWARE=1 silences benign Mesa/EGL GPU warnings in WSL2)"
