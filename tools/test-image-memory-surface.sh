#!/bin/sh
# Run from any directory. Requires the game's SDL2/SDL2_image/SDL2_ttf, PNG and
# GLEW development packages, pkg-config, and a compiler with section GC support.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT HUP INT TERM
cd "$root"
${CXX:-c++} -std=c++11 -ffunction-sections -fdata-sections \
    -Isrc -Isrc/utils/src \
    $(pkg-config --cflags sdl2 SDL2_image SDL2_ttf libpng glew) \
    tests/image_memory_surface.cpp src/engine/video/image_base.cpp \
    -Wl,--gc-sections $(pkg-config --libs sdl2 SDL2_image SDL2_ttf libpng glew) \
    -o "$build/image-memory-surface"
"$build/image-memory-surface" data/fonts/LinLibertine_aBS.ttf \
    data/fonts/LinBiolinum_RBah.ttf data/fonts/Berenika-Oblique.ttf
