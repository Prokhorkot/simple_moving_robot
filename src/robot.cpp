#include <optional>

#include "robot.hpp"
#include "playground.hpp"

namespace moving_robot {
    MoveResult Robot::try_move(Directions direction, const Playground& playground) {
        const Position prev = coords_;
        coords_ = playground.clip_to_bounds(coords_ + direction);

        std::optional<Position> expired;
        if (prev != coords_) {
            expired = tail_.append(prev);
        }

        return MoveResult{
            .position = coords_,
            .previous_position = prev,
            .expired_tail = expired,
        };
    }
    
    std::optional<Position> RobotTail::append(Position previous_position) {
        tail_.push_front(previous_position);

        if (tail_.size() <= size_) {
            return std::nullopt;
        }

        const Position expired = tail_.back();
        tail_.pop_back();
        return expired;
    }
}