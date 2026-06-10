#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
mkdir -p "$BUILD"

# Get zstd and liblzma flags via pkg-config if available
CFLAGS=(-std=c++20 -O2 -Wall -Wextra -I"$ROOT/core/include")
LIBS=()

if pkg-config --exists libzstd 2>/dev/null && pkg-config --exists liblzma 2>/dev/null; then
    CFLAGS+=($(pkg-config --cflags libzstd liblzma))
    LIBS+=($(pkg-config --libs libzstd liblzma))
    echo "Linking zstd and liblzma via pkg-config."
else
    # Fall back to common MSYS2/MinGW64 locations
    for MP in /mingw64 /mingw32 /msys64/mingw64 /c/msys64/mingw64; do
        if [[ -f "$MP/lib/libzstd.dll.a" ]]; then
            CFLAGS+=("-I$MP/include")
            LIBS+=("-L$MP/lib" -lzstd -llzma)
            echo "Using MSYS2 at $MP"
            break
        fi
    done
    if [[ ${#LIBS[@]} -eq 0 ]]; then
        echo "Warning: libzstd+liblzma not found; zstd/lzma2 compression disabled" >&2
    fi
fi

echo "Building setup-template.exe ..."
g++ "${CFLAGS[@]}" \
    "$ROOT/core/src/RepackCore.cpp" \
    "$ROOT/core/src/RepackCoreCompression.cpp" \
    "$ROOT/apps/InstallerApp/src/main.cpp" \
    -o "$BUILD/setup-template.exe" \
    "${LIBS[@]}"

echo "Building packer.exe ..."
g++ "${CFLAGS[@]}" \
    "$ROOT/core/src/RepackCore.cpp" \
    "$ROOT/core/src/RepackCoreCompression.cpp" \
    "$ROOT/apps/PackerApp/src/main.cpp" \
    -o "$BUILD/packer.exe" \
    "${LIBS[@]}"

echo "Build complete:"
echo "  $BUILD/packer.exe"
echo "  $BUILD/setup-template.exe"
