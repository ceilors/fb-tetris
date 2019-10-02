#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include "structs.hpp"

#include <png.h>

class Image {
public:
    Image(const char *image);
    ~Image();

    Color get_pixel(uint32_t x, uint32_t y) const;

    png_byte *raw;
    uint32_t width;
    uint32_t height;
    uint8_t color_channels;
};

#endif