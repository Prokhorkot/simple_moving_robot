#pragma once
#include <cstddef>
#include <vector>

#include "position.hpp"

namespace moving_robot {
    struct PlaygroundSize {
        int width;
        int height;
    };

    class Playground {
        public:
            Playground(int width, int height);
            Position clip_to_bounds(Position position) const;
            bool is_free(int x, int y) const;

        private:
            enum class Cell : unsigned char {
                Free,
                Occupied,
            };

            std::size_t index(int x, int y) const;

            PlaygroundSize size;
            std::vector<Cell> world_;
    };
}
