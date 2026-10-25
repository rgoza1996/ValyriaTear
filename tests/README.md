# ImageMemory SDL surface regression

Run `tools/test-image-memory-surface.sh` from any directory. It compiles the
production `image_base.cpp` with an OpenGL upload stub, so it needs the game's
SDL2, SDL2_image, SDL2_ttf, PNG and GLEW development packages, pkg-config, and a
compiler/linker supporting `-ffunction-sections` and `--gc-sections`. It does not
need an X server or an OpenGL context.

Four synthetic surfaces cover padded and packed 24-bit RGB and 32-bit RGBA rows.
The RGBA pixels have distinct color and alpha bytes. The test verifies every
uploaded pixel, dimensions and format, and verifies that copying leaves the
source pixels and padding unchanged. The previous contiguous SDL-surface copy
fails the first padded-row case.

Thirty-six additional cases render four strings (narrow glyphs, wide glyphs,
`DEUTSCH`, and accented/Greek characters) at three sizes through each of the
three bundled font families used for titles, body text and map headings. Their
actual SDL_ttf surface rows must reach the upload unchanged, with padding
excluded. Run with `LD_PRELOAD` unset to exercise the system SDL_ttf directly.

The stub checks the bytes supplied to OpenGL; it does not emulate rasterization
or exercise the two immediate TextSupervisor draw overloads. Those paths should
also be checked in the running game, especially growing dialogue text and a
second frame with unchanged text dimensions.
