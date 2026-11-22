#pragma once

#include <optional>

#include "directions.hpp"
#include "position.hpp"

class Robot {
    public:
        struct InputResult {
            bool keep_running;
            std::optional<Directions> direction;
        };

        explicit Robot(Position preferred_spawn = {0, 0});
        Position preferred_spawn() const;
        InputResult handle_input(int key) const;

    private:
        Position preferred_spawn_;  
};
