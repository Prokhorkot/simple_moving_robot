#pragma once

#include <optional>
#include <vector>

using ObjectID = int;

namespace moving_robot {
    enum class ObjectType {
        Unknown,
        Robot,
    };

    struct CellSnapshot {
        std::optional<ObjectID> occupant;
        std::vector<ObjectID> trail_robot_ids;
    };

    struct PlaygroundSnapshot {
        int width;
        int height;
        std::vector<CellSnapshot> cells;
    };
}
