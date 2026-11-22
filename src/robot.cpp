#include "robot.hpp"

#include <ncurses.h>

Robot::Robot(Position preferred_spawn): preferred_spawn_{preferred_spawn} {}

Position Robot::preferred_spawn() const {
    return preferred_spawn_;
}

Robot::InputResult Robot::handle_input(int key) const {
    switch (key) {
        case 'q':
        case 'Q':
        case '\x1b':
            return {false, std::nullopt};
        case KEY_UP: return {true, Directions::Up};
        case KEY_LEFT: return {true, Directions::Left};
        case KEY_DOWN: return {true, Directions::Down};
        case KEY_RIGHT: return {true, Directions::Right};
        default: return {true, std::nullopt};
    }
}
