#include "playground.hpp"

#include <algorithm>

namespace moving_robot {
    Position Playground::clip_to_bounds(Position position) const {
        return {
            std::clamp(position.x, 0, size.width - 1),
            std::clamp(position.y, 0, size.height - 1)
        };
    }
}
