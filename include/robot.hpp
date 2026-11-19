#pragma once

#include <deque>
#include <optional>

#include "directions.hpp"
#include "position.hpp"

namespace moving_robot {
    class Playground;

    struct MoveResult {
        Position position;
        Position previous_position;
        std::optional<Position> expired_tail;
    };

    class RobotTail {
        public:
            RobotTail(uint n): size_{n} {}
            std::optional<Position> append(Position previous_position);
        private:
            uint size_;
            std::deque<Position> tail_;
    };

    class Robot {
        public:
            Robot(uint tail_size): tail_{tail_size} {};
            MoveResult try_move(Directions direction, const Playground& playground);
            Position get_coords() { return coords_; };

        private:
            Position coords_{0, 0};
            Position direction_{0, 0};
            RobotTail tail_{3};
    };
}