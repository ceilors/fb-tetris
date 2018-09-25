#include "font.hpp"

Font::Font(const char * filename, uint16_t size) {
    font_size = size;

    FT_Init_FreeType(&library);
    FT_New_Face(library, filename, 0, &face);
    FT_Set_Char_Size(face, size * size, 0, font_dpi, 0);
}

Font::~Font() {
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void Font::render(FrameBuffer & fb, Point pos, const char * text) {
    slot = face->glyph;
    for (uint32_t n = 0; n < strlen(text); n++) {
        if (text[n] == ' ') {
            pos.x += font_size / 2;
            continue;
        }
        // load
        if (FT_Load_Char(face, text[n], FT_LOAD_RENDER)) {
            // ignore errors
            continue;
        }
        // draw
        for (uint32_t y = 0; y < slot->bitmap.rows; y++) {
            for (uint32_t x = 0; x < slot->bitmap.width; x++) {
                auto value = slot->bitmap.buffer[y * slot->bitmap.width + x];
                fb.draw_pixel({pos.x + x, pos.y + y - slot->bitmap_top}, {value, value, value});
            }
        }
        // move
        pos.x += slot->bitmap.width + (slot->bitmap.width / 3);
    }
}

void Font::render(FrameBuffer & fb, Point pos, uint32_t value) {
    std::ostringstream buffer;
    buffer << value;
    render(fb, pos, buffer.str().c_str());
}