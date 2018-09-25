#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <unistd.h>
#include <termios.h>
#include <functional>
#include "framebuffer.hpp"

class Window {
    std::function<void()> init_func = {};
    std::function<void(FrameBuffer &)> render_func = {};
    std::function<void(Window *, char)> keyboard_func = {};
    termios orig_term, raw_term;
    bool quit_flag = false;

    void set_stdin();
    void unset_stdin();
public:
    Window() {}

    void register_init(std::function<void()> init);
    void register_render(std::function<void(FrameBuffer &)> render);
    void register_keyboard(std::function<void(Window *, char)> keyboard);

    void main_loop();
    void send_close();
};

#endif