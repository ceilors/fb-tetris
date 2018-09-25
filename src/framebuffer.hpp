#ifndef __FRAMEBUFFER_HPP__
#define __FRAMEBUFFER_HPP__

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <setjmp.h>
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "image.hpp"
#include "structs.hpp"

class FrameBuffer {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char * draw_buffer;
    char * screen_buffer;
    long int screen_size;
    int fbfd;
    uint8_t buffer_index = 0;

    void draw_line_low(Rect l, Color color);
    void draw_line_high(Rect l, Color color);
public:
    FrameBuffer(const char *device);
    ~FrameBuffer();

    void draw_pixel(Point point, Color color);
    void draw_rectangle(Rect rect, Color color);
    void draw_image(Point point, const Image & image);

    void draw_line(Rect l, Color color);
    void draw_framed_rect(Rect l, Color color);
    void draw_image_rect(Rect wnd, Rect pos, const Image & image);

    void clear_screen(Color color);

    void swap_buffer();
};

#endif