#include <ncurses.h>

#include <optional>

#include "robot.hpp"
#include "playground.hpp"


int main() {
    moving_robot::Playground playground{100, 100};
    moving_robot::Robot robot{10};
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    const int pad_y = 1;
    const moving_robot::Position initial_position = robot.get_coords();
    moving_robot::MoveResult move_result{
        .position = initial_position,
        .previous_position = initial_position,
        .expired_tail = std::nullopt,
    };

    while (true) {
        const moving_robot::Position& position = move_result.position;
        const std::optional<moving_robot::Position>& expired_tail = move_result.expired_tail;

        if (expired_tail) {
            mvaddch(expired_tail->y + pad_y, expired_tail->x, ' ');
        }

        const moving_robot::Position& previous = move_result.previous_position;
        mvaddch(previous.y + pad_y, previous.x, '.');
        mvaddch(position.y + pad_y, position.x, 'R');
        mvprintw(0, 0, "Robot position: (%2d, %2d)  [q to quit]", position.x, position.y);
        move(position.y + pad_y, position.x);
        refresh();

        const int key = getch();
        if (key == 'q' || key == '\x1b' || key == ERR) {
            break;
        }

        switch (key)
        {
            case KEY_UP: move_result = robot.try_move(moving_robot::Directions::Up, playground); break;
            case KEY_LEFT: move_result = robot.try_move(moving_robot::Directions::Left, playground); break;
            case KEY_DOWN: move_result = robot.try_move(moving_robot::Directions::Down, playground); break;
            case KEY_RIGHT: move_result = robot.try_move(moving_robot::Directions::Right, playground); break;
        }
    }
    endwin();
    return 0;
}