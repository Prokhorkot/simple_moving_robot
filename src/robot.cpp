#include <optional>

#include "robot.hpp"

namespace moving_robot {
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
