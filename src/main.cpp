#include <ncurses.h>

#include <functional>

#include "playground.hpp"
#include "renderer.hpp"
#include "robot.hpp"

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

namespace {
    constexpr int width = 100;
    constexpr int height = 100;
    using ConsolePlayground = Playground<width, height>;

    bool handle_input(ConsolePlayground& playground, ObjectID robot_id) {
        const int key = getch();
        if (key == 'q' || key == '\x1b' || key == ERR) {
            return false;
        }

        Directions direction;
        switch (key)
        {
            case KEY_UP: direction = Directions::Up; break;
            case KEY_LEFT: direction = Directions::Left; break;
            case KEY_DOWN: direction = Directions::Down; break;
            case KEY_RIGHT: direction = Directions::Right; break;
            default: return true;
        }

        playground.move_robot(robot_id, direction);
        playground.next_tick();
        return true;
    }
}

int main() {
    const int trail_length = 10;
    ConsolePlayground playground{trail_length + 1};
    const ObjectID robot_id = playground.register_robot(Robot{});
    if (robot_id == -1) {
        return 1;
    }
    playground.next_tick();
    Terminal terminal;
    const Renderer renderer;
    const std::function<ObjectType(ObjectID)> object_type =
        std::bind_front(&ConsolePlayground::object_type, &playground);

    do {
        renderer.draw_frame(playground.snapshot(), object_type);
    } while (handle_input(playground, robot_id));

    return 0;
}
