#include "playground.hpp"

#include <algorithm>
#include <cstddef>

namespace moving_robot {
    Playground::Playground(int width, int height): size{width, height} {
        const std::size_t columns = static_cast<std::size_t>(width);
        const std::size_t rows = static_cast<std::size_t>(height);
        world_.resize(rows * columns, Cell::Free);
    }

    Position Playground::clip_to_bounds(Position position) const {
        return {
            std::clamp(position.x, 0, size.width - 1),
            std::clamp(position.y, 0, size.height - 1)
        };
    }

    bool Playground::is_free(int x, int y) const {
        if (x < 0 || x >= size.width || y < 0 || y >= size.height) {
            return false;
        }

        return world_[index(x, y)] == Cell::Free;
    }

    std::size_t Playground::index(int x, int y) const {
        const std::size_t row = static_cast<std::size_t>(y);
        const std::size_t width = static_cast<std::size_t>(size.width);
        const std::size_t column = static_cast<std::size_t>(x);
        return row * width + column;
    }
}
