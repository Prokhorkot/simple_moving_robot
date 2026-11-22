#include "robot.hpp"

#include <algorithm>
#include <limits>
#include <random>
#include <utility>

template <int Width, int Height>
Playground<Width, Height>::Playground(int max_traceback): max_traceback_{max_traceback} {}

template <int Width, int Height>
Position Playground<Width, Height>::clip_to_bounds(Position position) const {
    return {
        std::clamp(position.x, 0, Width - 1),
        std::clamp(position.y, 0, Height - 1)
    };
}

template <int Width, int Height>
ObjectID Playground<Width, Height>::register_robot(const Robot& robot) {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    Position start = robot.preferred_spawn();
    if (!is_free_unlocked(start.x, start.y)) {
        // Each registered robot occupies exactly one cell.
        if (position_by_objectid.size() >= world_.size()) {
            return -1;
        }

        std::mt19937 generator{std::random_device{}()};
        std::uniform_int_distribution<int> pick_x(0, Width - 1);
        std::uniform_int_distribution<int> pick_y(0, Height - 1);
        do {
            start = Position{pick_x(generator), pick_y(generator)};
        } while (!is_free_unlocked(start.x, start.y));
    }

    if (last_robot_id_ == std::numeric_limits<ObjectID>::max()) {
        return -1;
    }

    const ObjectID id = last_robot_id_ + 1;
    position_by_objectid.emplace(id, start);
    last_robot_id_ = id;
    world_[index(start.x, start.y)].occupant = id;
    return id;
}

template <int Width, int Height>
bool Playground<Width, Height>::remove_robot(ObjectID robot_id) {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const auto robot = position_by_objectid.find(robot_id);
    if (robot == position_by_objectid.end()) {
        return false;
    }

    const Position position = robot->second;
    world_[index(position.x, position.y)].occupant.reset();
    position_by_objectid.erase(robot);
    for (Cell& cell : world_) {
        cell.visits.erase(robot_id);
    }
    std::erase_if(command_queue_, [robot_id](const MoveCommand& command) {
        return command.robot_id == robot_id;
    });
    return true;
}

template <int Width, int Height>
bool Playground<Width, Height>::move_robot(ObjectID robot_id, Directions delta) {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    if (!position_by_objectid.contains(robot_id)) {
        return false;
    }
    command_queue_.push_back(MoveCommand{robot_id, delta});
    return true;
}

template <int Width, int Height>
bool Playground<Width, Height>::is_free(int x, int y) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    return is_free_unlocked(x, y);
}

template <int Width, int Height>
bool Playground<Width, Height>::next_tick() {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const bool had_commands = !command_queue_.empty();

    std::vector<Occupant> snapshot;
    snapshot.reserve(world_.size());
    for (const Cell& cell : world_) {
        snapshot.push_back(cell.occupant);
    }

    std::vector<Occupant> next_occupants = snapshot;
    execute_commands(snapshot, next_occupants);
    const bool trail_changed = update_grid(next_occupants);
    return had_commands || trail_changed;
}

template <int Width, int Height>
void Playground<Width, Height>::execute_commands(
    const std::vector<Occupant>& snapshot,
    std::vector<Occupant>& next_occupants
) {
    while (!command_queue_.empty()) {
        const MoveCommand command = command_queue_.front();
        command_queue_.pop_front();
        Position& position = position_by_objectid.at(command.robot_id);
        const Position next = position + command.direction;
        if (!contains(next.x, next.y)) {
            continue;
        }

        const std::size_t destination = index(next.x, next.y);
        if (snapshot[destination] || next_occupants[destination]) {
            continue;
        }

        next_occupants[index(position.x, position.y)].reset();
        next_occupants[destination] = command.robot_id;
        position = next;
    }
}

template <int Width, int Height>
bool Playground<Width, Height>::update_grid(const std::vector<Occupant>& next_occupants) {
    for (std::size_t cell = 0; cell < world_.size(); ++cell) {
        world_[cell].occupant = next_occupants[cell];
    }
    if (max_traceback_ <= 0) {
        return false;
    }
    bool trail_changed = false;
    for (Cell& cell : world_) {
        if (cell.occupant) {
            cell.visits.try_emplace(*cell.occupant, max_traceback_);
        }
        for (std::pair<const ObjectID, VisionInfo>& visit : cell.visits) {
            const bool was_trail = visit.second.last_visited() > 0;
            visit.second.update(cell.occupant == visit.first);
            const bool is_trail = visit.second.last_visited() > 0;
            trail_changed |= was_trail != is_trail;
        }
    }
    return trail_changed;
}

template <int Width, int Height>
PlaygroundSnapshot Playground<Width, Height>::snapshot() const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    PlaygroundSnapshot snapshot{Width, Height, {}};
    snapshot.cells.reserve(world_.size());
    for (const Cell& cell : world_) {
        CellSnapshot& copied_cell = snapshot.cells.emplace_back(CellSnapshot{cell.occupant, {}});
        for (const std::pair<const ObjectID, VisionInfo>& visit : cell.visits) {
            if (visit.second.last_visited() > 0) {
                copied_cell.trail_robot_ids.push_back(visit.first);
            }
        }
    }
    return snapshot;
}

template <int Width, int Height>
ObjectType Playground<Width, Height>::object_type(ObjectID id) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    return position_by_objectid.contains(id) ? ObjectType::Robot : ObjectType::Unknown;
}

template <int Width, int Height>
std::optional<Position> Playground<Width, Height>::robot_position(ObjectID robot_id) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const std::unordered_map<ObjectID, Position>::const_iterator robot =
        position_by_objectid.find(robot_id);
    if (robot == position_by_objectid.end()) {
        return std::nullopt;
    }
    return robot->second;
}

template <int Width, int Height>
int Playground<Width, Height>::last_visited(ObjectID robot_id, const Position& position) const {
    if (!contains(position.x, position.y)) {
        return -1;
    }

    const std::lock_guard<std::mutex> lock(actions_mutex);
    const Cell& cell = world_[index(position.x, position.y)];
    const std::unordered_map<ObjectID, VisionInfo>::const_iterator visit = cell.visits.find(robot_id);
    if (visit == cell.visits.end()) {
        return -1;
    }

    return visit->second.last_visited();
}

template <int Width, int Height>
bool Playground<Width, Height>::contains(int x, int y) const {
    return x >= 0 && x < Width && y >= 0 && y < Height;
}

template <int Width, int Height>
bool Playground<Width, Height>::is_free_unlocked(int x, int y) const {
    return contains(x, y) && !world_[index(x, y)].occupant;
}

template <int Width, int Height>
std::size_t Playground<Width, Height>::index(int x, int y) const {
    const std::size_t row = static_cast<std::size_t>(y);
    const std::size_t width = static_cast<std::size_t>(Width);
    const std::size_t column = static_cast<std::size_t>(x);
    return row * width + column;
}
