#pragma once

#include <string>
#include "directions.hpp"

namespace moving_robot {
    class Playground;

    struct Position {
        int x;
        int y;
    };

    std::string to_string(const Position& position);

    class Robot {
        public:
            Position try_move(Directions direction, const Playground& playground);
            Position get_coords() { return coords_; };

        private:
            Position coords_{0, 0};
            Position change_coords(int dx, int dy);
    };
}