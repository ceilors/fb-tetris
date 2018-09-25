#include "window.hpp"
#include "core.hpp"

Tetris tetris = Tetris();

void keyboard(Window * w, char key) {
    switch (key) {
        case 'w': {
            tetris.move(MOVE_HARD_DOWN);
            break;
        }
        case 's': {
            tetris.move(MOVE_SOFT_DOWN);
            break;
        }
        case 'a': {
            tetris.move(MOVE_LEFT);
            break;
        }
        case 'd': {
            tetris.move(MOVE_RIGHT);
            break;
        }
        case 'g': {
            tetris.move(ROTATE_LEFT);
            break;
        }
        case 'h': {
            tetris.move(ROTATE_RIGHT);
            break;
        }
        case ' ': {
            tetris.move(PAUSE_STATE);
            break;
        }
        case 'q': {
            w->send_close();
            break;
        }
        default: {
            break;
        }
    }
}

void render(FrameBuffer & fb) {
    tetris.render(fb);
    tetris.move(MOVE_SOFT_DOWN, true);
}

int main() {
    Window w;

    w.register_keyboard(keyboard);
    w.register_render(render);

    w.main_loop();

    return EXIT_SUCCESS;
}
