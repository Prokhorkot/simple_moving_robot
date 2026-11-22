#include "position.hpp"

#include <array>

std::string to_string(const Position& position) {
    return "(" + std::to_string(position.x) + ", "
               + std::to_string(position.y) + ")";
}

bool Position::operator==(const Position&) const = default;
bool Position::operator!=(const Position&) const = default;


Position Position::operator+(Directions direction) const {
    constexpr std::array offsets{
        Position{ 0, -1},  // Up
        Position{-1,  0},  // Left
        Position{ 0,  1},  // Down
        Position{ 1,  0},  // Right
    };

    const Position offset = offsets.at(static_cast<int>(direction));
    return {x + offset.x, y + offset.y};
}
