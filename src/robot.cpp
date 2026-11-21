#include "robot.hpp"

namespace moving_robot {
    Robot::Robot(Position preferred_spawn): preferred_spawn_{preferred_spawn} {}

    Position Robot::preferred_spawn() const {
        return preferred_spawn_;
    }
}
