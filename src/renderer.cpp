#include "renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <ncurses.h>

void Renderer::draw_frame(
    const PlaygroundSnapshot& snapshot,
    const char* status
) const {
    constexpr int pad_y = 1;
    int rows;
    int columns;
    getmaxyx(stdscr, rows, columns);
    const int visible_width = std::min(snapshot.width, columns);
    const int visible_height = std::min(snapshot.height, rows - pad_y);

    erase();
    for (int y = 0; y < visible_height; ++y) {
        const std::size_t row = static_cast<std::size_t>(y) *
            static_cast<std::size_t>(snapshot.width);
        for (int x = 0; x < visible_width; ++x) {
            const CellSnapshot& cell = snapshot.cells[row + static_cast<std::size_t>(x)];
            if (cell.occupant) {
                mvaddch(y + pad_y, x, symbol_for(cell.occupant_type));
            } else if (!cell.trail_robot_ids.empty()) {
                mvaddch(y + pad_y, x, '.');
            }
        }
    }
    mvaddstr(0, 0, status);
    refresh();
}

chtype Renderer::symbol_for(ObjectType type) {
    switch (type) {
        case ObjectType::Robot:
            return has_colors() ? ('H' | COLOR_PAIR(1)) : 'H';
        case ObjectType::Barrier: return '#';
        default: return '?';
    }
}
