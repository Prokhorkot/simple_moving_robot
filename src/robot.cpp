#include <string>

#include "robot.hpp"
#include "playground.hpp"

namespace moving_robot {
    std::string to_string(const Position& position) {
        return "(" + std::to_string(position.x) + ", "
                   + std::to_string(position.y) + ")";
    };

    Position Robot::try_move(Directions direction, const Playground& playground) {
        Position next = coords_;

       switch (direction) {
        case Directions::Up:
            next = change_coords(0, -1);
            break;
        
        case Directions::Left:
            next = change_coords(-1, 0);
            break;

        case Directions::Down:
            next = change_coords(0, 1);
            break;

        case Directions::Right:
            next = change_coords(1, 0);
            break;
       }

       next = playground.clip_to_bounds(next);
       coords_ = next;
       return next;
    };

    Position Robot::change_coords(int dx, int dy) {
        return Position{coords_.x + dx, coords_.y + dy};
    }
}