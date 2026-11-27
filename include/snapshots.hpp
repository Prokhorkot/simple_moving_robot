#pragma once

#include <cstdint>
#include <optional>
#include <vector>

using ObjectID = int;

enum class ObjectType : std::uint8_t {
    Unknown = 0,
    Robot = 1,
    Barrier = 2,
};

struct CellSnapshot {
    std::optional<ObjectID> occupant;
    std::vector<ObjectID> trail_robot_ids;
    ObjectType occupant_type = ObjectType::Unknown;
};

struct PlaygroundSnapshot {
    int width;
    int height;
    std::vector<CellSnapshot> cells;
};
