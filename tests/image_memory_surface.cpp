// Exercise the production SDL-surface copy without requiring an OpenGL context.
// The upload stub captures the exact bytes handed to OpenGL.
#include <SDL.h>
#include <SDL_ttf.h>
#include "engine/video/image_base.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
std::vector<uint8_t> uploaded;
GLenum uploaded_format;
int uploaded_width, uploaded_height;

void CheckSurface(SDL_Surface* surface, const char* name)
{
    if (!surface)
        throw std::runtime_error(SDL_GetError());
    const size_t row_bytes = surface->w * surface->format->BytesPerPixel;
    std::vector<uint8_t> expected;
    for (int y = 0; y < surface->h; ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(surface->pixels) + y * surface->pitch;
        expected.insert(expected.end(), row, row + row_bytes);
    }
    const std::vector<uint8_t> original(static_cast<const uint8_t*>(surface->pixels),
                                      static_cast<const uint8_t*>(surface->pixels) + surface->h * surface->pitch);
    vt_video::private_video::ImageMemory image(surface);
    image.GlTexSubImage(0, 0);
    if (image.GetWidth() != static_cast<size_t>(surface->w) ||
        image.GetHeight() != static_cast<size_t>(surface->h) ||
        image.GetBytesPerPixel() != surface->format->BytesPerPixel ||
        uploaded_width != surface->w || uploaded_height != surface->h ||
        uploaded_format != (surface->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA) ||
        uploaded != expected)
        throw std::runtime_error(std::string(name) + ": packed pixel bytes differ");
    if (std::memcmp(original.data(), surface->pixels, original.size()) != 0)
        throw std::runtime_error(std::string(name) + ": input surface changed");
    std::cout << name << ": " << surface->w << 'x' << surface->h
              << " bpp=" << unsigned(surface->format->BytesPerPixel)
              << " pitch=" << surface->pitch << " row=" << row_bytes << " PASS\n";
}

void CheckSynthetic(int width, int height, int pitch, Uint32 format, const char* name)
{
    std::vector<uint8_t> pixels(height * pitch, 0xED);
    const int bpp = SDL_BYTESPERPIXEL(format);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width * bpp; ++x)
            pixels[y * pitch + x] = static_cast<uint8_t>((y * 41 + x * 17) % 251);
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), width, height,
                                                             bpp * 8, pitch, format);
    CheckSurface(surface, name);
    SDL_FreeSurface(surface);
}
}

extern "C" void GLAPIENTRY glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei width,
                                           GLsizei height, GLenum format, GLenum type,
                                           const void* pixels)
{
    if (type != GL_UNSIGNED_BYTE)
        throw std::runtime_error("unexpected upload type");
    uploaded_width = width;
    uploaded_height = height;
    uploaded_format = format;
    const uint8_t* bytes = static_cast<const uint8_t*>(pixels);
    uploaded.assign(bytes, bytes + width * height * (format == GL_RGB ? 3 : 4));
}

int main(int argc, char** argv)
{
    try {
        CheckSynthetic(3, 5, 32, SDL_PIXELFORMAT_RGBA32, "padded RGBA distinct channels and alpha");
        CheckSynthetic(4, 3, 16, SDL_PIXELFORMAT_RGBA32, "tight RGBA");
        CheckSynthetic(3, 5, 12, SDL_PIXELFORMAT_RGB24, "padded RGB");
        CheckSynthetic(4, 3, 12, SDL_PIXELFORMAT_RGB24, "tight RGB");
        if (TTF_Init() != 0)
            throw std::runtime_error(TTF_GetError());
        const Uint16 samples[][32] = {{'i', 'l', '1', 0}, {'W', 'W', 'M', ' ', '8', '8', 0},
                                     {'D', 'E', 'U', 'T', 'S', 'C', 'H', 0},
                                     {'R', 0x00e9, 's', 'u', 'm', 0x00e9, ' ', 0x03a9, 0}};
        for (int f = 1; f < argc; ++f) {
            for (int size : {18, 22, 30}) {
                TTF_Font* font = TTF_OpenFont(argv[f], size);
                if (!font)
                    throw std::runtime_error(TTF_GetError());
                for (const auto& sample : samples) {
                    SDL_Surface* surface = TTF_RenderUNICODE_Blended(font, sample, SDL_Color{255, 255, 255, 255});
                    CheckSurface(surface, argv[f]);
                    SDL_FreeSurface(surface);
                }
                TTF_CloseFont(font);
            }
        }
        TTF_Quit();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
