#include "window.hpp"

void Window::set_stdin() {
    tcgetattr(STDIN_FILENO, &orig_term);

    raw_term = orig_term;
    raw_term.c_lflag &= ~(ECHO | ICANON);
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);
}

void Window::unset_stdin() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

void Window::register_init(std::function<void()> init) {
    init_func = init;
}

void Window::register_render(std::function<void(FrameBuffer &)> render) {
    render_func = render;
}

void Window::register_keyboard(std::function<void(Window *, char)> keyboard) {
    keyboard_func = keyboard;
}

void Window::main_loop() {
    if (init_func) {
        init_func();
    }
    set_stdin();

    FrameBuffer fb = FrameBuffer("/dev/fb0");

    fb.clear_screen({0, 0, 0});

    while (!quit_flag) {
        char input_key;
        int readed = read(STDIN_FILENO, &input_key, 1);
        if (readed > 0 && keyboard_func) {
            keyboard_func(this, input_key);
        }
        if (render_func) {
            render_func(fb);
            fb.swap_buffer();
        }
        usleep(1E4);
    }

    unset_stdin();
}

void Window::send_close() {
    quit_flag = true;
}