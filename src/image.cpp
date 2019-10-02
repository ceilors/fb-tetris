#include "image.hpp"

Image::Image(const char *image) {
    color_channels = 0;
    png_byte header[8];
    FILE *f;

    if (!(f = fopen(image, "r"))) {
        throw std::runtime_error("fopen problem");
    }
    fread(header, 1, 8, f);
    int is_png = !png_sig_cmp(header, 0, 8);
    if (!is_png) {
        fclose(f);
        throw std::runtime_error("is not png");
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(f);
        throw std::runtime_error("png_create_read_struct problem");
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(f);
        throw std::runtime_error("png_create_info_struct problem");
    }
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(f);
        throw std::runtime_error("png_create_info_struct problem");
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(f);
        throw std::runtime_error("setjmp problem");
    }

    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, NULL, NULL, NULL, NULL, NULL);

    switch (png_get_color_type(png_ptr, info_ptr)) {
        case PNG_COLOR_TYPE_GRAY: {
            png_set_gray_to_rgb(png_ptr);
            color_channels = 3;
            break;
        }
        case PNG_COLOR_TYPE_GRAY_ALPHA: {
            png_set_gray_to_rgb(png_ptr);
            color_channels = 4;
            break;
        }
        case PNG_COLOR_TYPE_PALETTE: {
            png_set_palette_to_rgb(png_ptr);
            color_channels = 3;
            break;
        }
        case PNG_COLOR_TYPE_RGB: {
            color_channels = 3;
            break;
        }
        case PNG_COLOR_TYPE_RGB_ALPHA: {
            color_channels = 4;
            break;
        }
        default: {
            break;
        }
    }

    png_read_update_info(png_ptr, info_ptr);
    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    raw = new png_byte[row_bytes * height];
    if (!raw) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(f);
        throw std::runtime_error("empty raw data");
    }
    png_bytepp row_pointers = new png_bytep[height];
    if (!row_pointers) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        delete[] raw;
        fclose(f);
        throw std::runtime_error("row_pointers problem");
    }
    for (uint32_t i = 0; i < height; ++i) {
        row_pointers[i] = raw + i * row_bytes;
    }
    png_read_image(png_ptr, row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] row_pointers;
    fclose(f);
}

Image::~Image() { delete[] raw; }

Color Image::get_pixel(uint32_t x, uint32_t y) const {
    uint32_t location = (y * width + x) * color_channels;
    return {raw[location + 0], raw[location + 1], raw[location + 2]};
}