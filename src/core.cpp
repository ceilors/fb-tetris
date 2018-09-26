#include "core.hpp"

// simple random
static uint8_t y8 = (uint8_t)time(NULL);

static uint8_t tile_size = 32;

const uint8_t next_figure_blocks_count = 6;
const uint16_t score_table[5] = {0, 100, 300, 700, 1500};

const Point shift = {20, 20};

uint8_t xorshift8(void) {
    y8 ^= (y8 << 7);
    y8 ^= (y8 >> 5);
    return y8 ^= (y8 << 3);
}

point find_phantom(Field field, Figure curr) {
    Figure t = curr;
    while (!field.intersect(&t)) {
        t.pos.y--;
    }
    curr.pos.y = t.pos.y + 1;
    return curr.pos;
}

uint8_t & Field::operator () (uint8_t x, uint8_t y) {
    if (x < _width && y < _height) {
        return field[y * _width + x];
    } else {
        throw std::out_of_range("index out of range");
    }
}

void Field::set(Figure * f) {
    for (auto it : f->coords) {
        (*this)(it.x + f->pos.x, it.y + f->pos.y) = f->color + 1;
    }
}

bool Field::intersect(Figure * f) {
    // достижение дна
    for (auto it : f->coords) {
        // пересечение с фигурой
        try {
            if ((*this)(it.x + f->pos.x, it.y + f->pos.y)) {
                return true;
            }
        } catch (std::out_of_range &) { return true; }
    }
    return false;
}

void Field::clear_line(uint8_t index) {
    for (uint8_t y = index + 1; y < _height; ++y) {
        for (uint8_t x = 1; x < _width - 1; ++x) {
            (*this)(x, y - 1) = (*this)(x, y);
        }
    }
}

bool Field::check_line(uint8_t index) {
    uint8_t filled = 0;
    for (uint8_t x = 0; x < _width; ++x) {
        uint8_t value = 1 ? (*this)(x, index) > 0 : 0;
        filled += value;
    }
    return filled == _width;
}

void Field::clear() {
    for (auto & i : field) {
        i = 0;
    }
    for (uint8_t y = 0; y < _height; ++y) {
        (*this)(0, y) = 1;
        (*this)(_width - 1, y) = 1;
    }
    for (uint8_t x = 0; x < _width; ++x) {
        (*this)(x, 0) = 1;
    }
}

void Figure::set(uint8_t figure, uint8_t w, uint8_t h) {
    x_max = y_max = 0;
    color = figure + 1;
    coords.clear();
    switch (figure) {
        case FIGURE_I:
            coords = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
            break;
        case FIGURE_J:
            coords = {{0, 0}, {0, 1}, {1, 1}, {2, 1}};
            break;
        case FIGURE_L:
            coords = {{2, 0}, {0, 1}, {1, 1}, {2, 1}};
            break;
        case FIGURE_O:
            coords = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
            break;
        case FIGURE_S:
            coords = {{0, 0}, {0, 1}, {1, 1}, {1, 2}};
            break;
        case FIGURE_T:
            coords = {{1, 0}, {0, 1}, {1, 1}, {2, 1}};
            break;
        case FIGURE_Z:
            coords = {{1, 0}, {0, 1}, {1, 1}, {0, 2}};
            break;
        default:
            throw std::invalid_argument("invalid figure index");
    }
    for (auto it : coords) {
        x_max = std::max(it.x, x_max);
        y_max = std::max(it.y, y_max);
    }
    pos = point((w - x_max) / 2, h - y_max);
}

void Figure::rotate(bool direction) {
    int8_t max_x = x_max;
    int8_t mx = max_x;
    int8_t my = max_x;
    // rotate
    for (auto & it : coords) {
        int8_t x = it.x;
        int8_t y = it.y;
        if (direction) {
            it.x = max_x - y;
            it.y = x;
        } else {
            it.x = y;
            it.y = max_x - x;
        }
        mx = std::min(it.x, mx);
        my = std::min(it.y, my);
    }
    // shift
    for (auto & it : coords) {
        it.x -= mx;
        it.y -= my;
    }
    // x & y maxs
    for (auto & it : coords) {
        x_max = std::max(it.x, x_max);
        y_max = std::max(it.y, y_max);
    }
}

void Tetris::gameover() {
    // game consts
    update_counter_max = 50;
    time_to_set_default = 40;
    // game timer
    time_to_set = time_to_set_default;
    update_counter = update_counter_max;

    // game info
    high_score = std::max(game_score, high_score);
    game_score = 0;
    game_speed = 1;
    game_lines = 0;

    game_over_flag = false;

    field.clear();
    curr.set(xorshift8() % (FIGURE_Z + 1), field.width(), field.height() - 1);
    next.set(xorshift8() % (FIGURE_Z + 1), field.width(), field.height() - 1);
}

void Tetris::move(uint8_t state, bool delay) {
    point _shifts[] = {point(-1, 0), point(1, 0), point(0, -1), point(0, 1)};
    figure_t shifts(_shifts, _shifts+4);
    static bool set_flag = false;

    if (state != PAUSE_STATE && pause_flag) {
        return;
    }

    switch (state) {
        case PAUSE_STATE: {
            pause_flag = !pause_flag;
            if (game_over_flag) {
                gameover();
            }
            return;
            break;
        }
        case MOVE_LEFT: {
            curr.pos.x--;
            if (field.intersect(&curr)) {
                curr.pos.x++;
            }
            break;
        }
        case MOVE_RIGHT: {
            curr.pos.x++;
            if (field.intersect(&curr)) {
                curr.pos.x--;
            }
            break;
        }
        case MOVE_SOFT_DOWN: {
            // задержка для регулирования скорости игры
            if (update_counter <= 0 || !delay || set_flag) {
                update_counter = update_counter_max;
            } else {
                update_counter--;
                return;
            }

            curr.pos.y--;
            if (field.intersect(&curr)) {
                curr.pos.y++;
                set_flag = true;
            }
            break;
        }
        case MOVE_HARD_DOWN: {
            point new_pos = find_phantom(field, curr);
            // очки за быстрый спуск
            game_score += (curr.pos.y - new_pos.y);
            curr.pos = new_pos;
            set_flag = true;
            break;
        }
        case ROTATE_LEFT:
        case ROTATE_RIGHT: {
            bool side = true ? state == ROTATE_LEFT : false;
            bool ignored_all = true;
            curr.rotate(side);
            if (field.intersect(&curr)) {
                for (int8_t k = 1; k <= std::max(curr.x_max, curr.y_max); ++k) {
                    for (auto it : shifts) {
                        point s = it;
                        curr.pos.x += s.x * k;
                        curr.pos.y += s.y * k;
                        if (!field.intersect(&curr)) {
                            ignored_all = false;
                            break;
                        } else {
                            curr.pos.x -= s.x * k;
                            curr.pos.y -= s.y * k;
                        }
                    }
                    if (!ignored_all) {
                        break;
                    }
                }
                if (ignored_all) {
                    curr.rotate(!side);
                }
            }
            break;
        }
        default:
            throw std::invalid_argument("invalid figure action");
    }
    if (set_flag && time_to_set <= 0 && !game_over_flag) {
        set_flag = false;
        time_to_set = time_to_set_default;
        try {
            field.set(&curr);
        } catch (std::out_of_range &) {
            game_over_flag = true;
            pause_flag = true;
            return;
        }
        // очки за установку фигуры на поле
        game_score += curr.coords.size();
        curr = next;
        next.set(xorshift8() % (FIGURE_Z + 1), field.width(), field.height() - 1);
        // очистка заполненных ячеек поля
        uint8_t lines_count = 0;
        for (int index = field.height() - 1; index >= 1; --index) {
            // здесь можно запилить подсчёт очков
            if (field.check_line((uint8_t)index)) {
                field.clear_line((uint8_t)index);
                lines_count++;
            }
        }
        // очки за удаление линий
        game_score += score_table[lines_count];
        if ((game_lines + lines_count) / 10 - game_lines / 10 == 1) {
            update_counter_max -= 4;
            time_to_set_default += 3;
            game_speed += 1;
        }
        game_lines += lines_count;
        // проверяем что фигуре ничего не мешает
        if (field.intersect(&curr)) {
            game_over_flag = true;
            pause_flag = true;
            return;
        }
    } else if (set_flag) {
        time_to_set--;
    }
}

Tetris::Tetris() {
    gameover();

    w_width = (field.width() + next_figure_blocks_count) * tile_size;
    w_height = field.height() * tile_size;
}

void draw_box(FrameBuffer & fb, Image & tex, int8_t x, int8_t y, uint8_t id, Point shift) {
    Rect wnd = { (uint32_t) id * tile_size, 0, tile_size, tile_size };
    Rect pos = { shift.x + x * tile_size, shift.y + y * tile_size, tile_size, tile_size };
    fb.draw_image_rect(wnd, pos, tex);
}

void draw_frame(FrameBuffer & fb, point * pos, figure_t & coords, Field * field, Point shift) {
    std::vector<Rect> lines;

    for (auto it : coords) {
        Point p0 = {(uint32_t) it.x * tile_size, (uint32_t) it.y * tile_size};
        Point p1 = {p0.x + tile_size, p0.y};
        Point p2 = {p1.x, p0.y + tile_size};
        Point p3 = {p0.x, p2.y};

        lines.push_back({p0.x, p0.y, p1.x, p1.y});
        lines.push_back({p3.x, p3.y, p2.x, p2.y});
        lines.push_back({p0.x, p0.y, p3.x, p3.y});
        lines.push_back({p1.x, p1.y, p2.x, p2.y});
    }

    for (auto it : lines) {
        int count = 0;
        for (auto p : lines) {
            if (p == it) {
                count++;
            }
        }
        if (count == 1) {
            Rect l = {
                shift.x + pos->x * tile_size + it.p1.x, shift.y + (field->height() - pos->y) * tile_size - it.p1.y,
                shift.x + pos->x * tile_size + it.p2.x, shift.y + (field->height() - pos->y) * tile_size - it.p2.y
            };
            fb.draw_line(l, {255, 255, 255});
        }
    }
}

void Tetris::render(FrameBuffer & fb) {
    const int8_t next_figure_shift = field.width() + 1;

    if (!pause_flag) {
        fb.draw_rectangle({0, 0, w_width + tile_size, w_height + tile_size}, {0, 0, 0});
        // рисуем игровое поле
        for (int8_t y = 0; y < field.height(); ++y) {
            for (int8_t x = 0; x < field.width(); ++x) {
                uint8_t tile_type = field(x, y);
                if (tile_type > 0) {
                    draw_box(fb, tile, x, field.height() - y - 1, tile_type - 1, shift);
                }
            }
        }
        // сдвигаем точку отчёта для оси y для человеческой отрисовки
        // рисуем текущую фигуру на поле
        for (auto it : curr.coords) {
            draw_box(fb, tile, it.x + curr.pos.x, field.height() - (it.y + curr.pos.y) - 1, curr.color, shift);
        }
        point phantom = find_phantom(field, curr);
        // рисуем предпологаемое место фигуры на дне стакана
        draw_frame(fb, &phantom, curr.coords, &field, shift);
        // следующая фигура
        for (auto it : next.coords) {
            draw_box(fb, tile, it.x + next_figure_shift, next.y_max - it.y + 1, next.color, shift);
        }
    } else {
        font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 15 * tile_size}, "paused");
    }
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y}, "next");
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 5 * tile_size}, "highscore");
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 6 * tile_size}, high_score);
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 7 * tile_size}, "score");
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 8 * tile_size}, game_score);
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 9 * tile_size}, "lines");
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 10 * tile_size}, game_lines);
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 12 * tile_size}, "speed");
    font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 13 * tile_size}, game_speed);
    if (game_over_flag) {
        font.render(fb, {shift.x + next_figure_shift * tile_size, shift.y + 16 * tile_size}, "gameover");
    }
}
