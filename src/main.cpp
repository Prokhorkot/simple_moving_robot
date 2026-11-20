#include <ncurses.h>

#include <optional>
#include <functional>

#include "robot.hpp"
#include "playground.hpp"

using namespace moving_robot;

class Terminal {
    public:
        Terminal() {
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
        }
        ~Terminal() { endwin(); }
};

int main() {
    Playground playground{100, 100};
    Robot robot{10};
    Terminal terminal;
    
    const int pad_y = 1;
    const Position initial_position = robot.get_coords();
    MoveResult move_result{
        .position = initial_position,
        .previous_position = initial_position,
        .expired_tail = std::nullopt,
    };

    while (true) {
        const Position& position = move_result.position;
        const std::optional<Position>& expired_tail = move_result.expired_tail;

        if (expired_tail) {
            mvaddch(expired_tail->y + pad_y, expired_tail->x, ' ');
        }

        const Position& previous = move_result.previous_position;
        mvaddch(previous.y + pad_y, previous.x, '.');
        mvaddch(position.y + pad_y, position.x, 'R');
        mvprintw(0, 0, "Robot position: (%2d, %2d)  [q to quit]", position.x, position.y);
        move(position.y + pad_y, position.x);
        refresh();

        const int key = getch();
        if (key == 'q' || key == '\x1b' || key == ERR) {
            break;
        }

        Directions direction;
        switch (key)
        {
            case KEY_UP: direction = Directions::Up; break;
            case KEY_LEFT: direction = Directions::Left; break;
            case KEY_DOWN: direction = Directions::Down; break;
            case KEY_RIGHT: direction = Directions::Right; break;
            default: continue;
        }

        move_result = robot.try_move(direction, std::bind_front(&Playground::is_free, &playground));
    }
    return 0;
}
