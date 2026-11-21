#pragma once

#include <functional>

#include "snapshots.hpp"

namespace moving_robot {
    class Renderer {
        public:
            void draw_frame(
                const PlaygroundSnapshot& snapshot,
                const std::function<ObjectType(ObjectID)>& object_type
            ) const;

        private:
            static char symbol_for(ObjectType type);
    };
}
