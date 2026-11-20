#pragma once

#include <concepts>
#include <deque>
#include <functional>
#include <optional>

#include "directions.hpp"
#include "position.hpp"

namespace moving_robot {
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
            template<typename F>
            requires std::predicate<F&, const int&, const int&>
            MoveResult try_move(Directions direction, F&& is_free) {
                const Position previous = coords_;
                const Position next = previous + direction;
                std::optional<Position> expired;

                if (is_free(next.x, next.y)) {
                    expired = tail_.append(previous);
                    coords_ = next;
                }

                return MoveResult{
                    .position = coords_,
                    .previous_position = previous,
                    .expired_tail = expired,
                };
            }
            Position get_coords() { return coords_; };

        private:
            Position coords_{0, 0};
            Position direction_{0, 0};
            RobotTail tail_{3};
    };
}
