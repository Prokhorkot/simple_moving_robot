#pragma once
#include "position.hpp"

namespace moving_robot {
    struct PlaygroundSize {
        int width;
        int height;
    };

    class Playground {
        public:
            Playground(int width, int height): size{width, height} {}
            Position clip_to_bounds(Position position) const;

        private:
            PlaygroundSize size{10, 10};
    };
}