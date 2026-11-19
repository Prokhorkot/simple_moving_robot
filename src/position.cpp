#include "position.hpp"

#include <array>

namespace moving_robot {
    std::string to_string(const Position& position) {
        return "(" + std::to_string(position.x) + ", "
                   + std::to_string(position.y) + ")";
    }

    Position Position::operator+(Directions direction) {
        constexpr std::array offsets{
            Position{ 0, -1},  // Up
            Position{-1,  0},  // Left
            Position{ 0,  1},  // Down
            Position{ 1,  0},  // Right
        };

        const auto offset = offsets.at(static_cast<int>(direction));
        return {x + offset.x, y + offset.y};
    }
}
