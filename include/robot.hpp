#pragma once

#include "position.hpp"

namespace moving_robot {
    class Robot {
        public:
            explicit Robot(Position preferred_spawn = {0, 0});
            Position preferred_spawn() const;

        private:
            Position preferred_spawn_;
    };
}
