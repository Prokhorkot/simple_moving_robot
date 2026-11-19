#pragma once

#include <string>

#include "directions.hpp"

namespace moving_robot {
    struct Position {
        int x;
        int y;
        Position operator+(Directions direction);
    };

    std::string to_string(const Position& position);
}
