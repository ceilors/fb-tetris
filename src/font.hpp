#ifndef __FONT_HPP__
#define __FONT_HPP__

#include <sstream>
#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "framebuffer.hpp"
#include "structs.hpp"


class Font {
    FT_Library library;
    FT_Face face;
    FT_GlyphSlot slot;

    const uint16_t font_dpi = 100;
    uint16_t font_size = 0;
public:
    Font(const char * filename, uint16_t size);
    ~Font();

    void render(FrameBuffer & fb, Point pos, const char * text);
    void render(FrameBuffer & fb, Point pos, uint32_t value);
};

#endif