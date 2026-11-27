#include "robot.hpp"

#include <algorithm>
#include <limits>
#include <random>
#include <utility>

template <int Width, int Height>
Playground<Width, Height>::Playground(int trail_lifetime_seconds, double barrier_probaility)
    : trail_lifetime_{trail_lifetime_seconds}, barrier_probaility_{barrier_probaility} {}

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
    if (!contains(start.x, start.y) || world_[index(start.x, start.y)].occupant) {
        std::vector<Position> available;
        for (int y = 0; y < Height; ++y) {
            for (int x = 0; x < Width; ++x) {
                if (!world_[index(x, y)].occupant) {
                    available.push_back(Position{x, y});
                }
            }
        }

        if (available.empty()) {
            return -1;
        }

        std::mt19937 generator{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> pick{0, available.size() - 1};
        start = available[pick(generator)];
    }

    const ObjectID id = generate_object_id();
    if (id != -1) {
        robot_positions_.emplace(id, start);
        world_[index(start.x, start.y)].occupant = id;
    }
    return id;
}

template <int Width, int Height>
ObjectID Playground<Width, Height>::generate_object_id() {
    if (last_object_id_ == std::numeric_limits<ObjectID>::max()) {
        return -1;
    }
    last_object_id_++;
    return last_object_id_;
}

template <int Width, int Height>
bool Playground<Width, Height>::remove_robot(ObjectID robot_id) {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    return remove_robot_locked(robot_id, /*clear_occupant=*/true);
}

template <int Width, int Height>
bool Playground<Width, Height>::remove_robot_locked(ObjectID robot_id, bool clear_occupant) {
    const auto robot = robot_positions_.find(robot_id);
    if (robot == robot_positions_.end()) {
        return false;
    }

    if (clear_occupant) {
        const Position position = robot->second;
        auto& occupant = world_[index(position.x, position.y)].occupant;
        if (occupant == robot_id) {
            occupant.reset();
        }
    }
    robot_positions_.erase(robot);
    for (Cell& cell : world_) {
        cell.clear_trail(robot_id);
    }
    std::erase_if(robot_move_queue_, [robot_id](const RobotMoveCommand& command) {
        return command.robot_id == robot_id;
    });
    return true;
}

template <int Width, int Height>
bool Playground<Width, Height>::queue_robot_move(ObjectID robot_id, Directions delta) {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    if (!robot_positions_.contains(robot_id)) {
        return false;
    }
    robot_move_queue_.push_back(RobotMoveCommand{robot_id, delta});
    return true;
}

template <int Width, int Height>
bool Playground<Width, Height>::is_free(int x, int y) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    return contains(x, y) && !world_[index(x, y)].occupant;
}

template <int Width, int Height>
bool Playground<Width, Height>::next_tick() {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const bool had_robot_moves = !robot_move_queue_.empty();

    std::vector<Occupant> snapshot;
    snapshot.reserve(world_.size());
    for (const Cell& cell : world_) {
        snapshot.push_back(cell.occupant);
    }

    std::vector<Occupant> next_occupants = snapshot;
    execute_robot_moves(snapshot, next_occupants);
    const bool trail_changed = commit_robot_tick(next_occupants);
    return had_robot_moves || trail_changed;
}

template <int Width, int Height>
void Playground<Width, Height>::execute_robot_moves(
    const std::vector<Occupant>& snapshot,
    std::vector<Occupant>& next_occupants
) {
    while (!robot_move_queue_.empty()) {
        const RobotMoveCommand command = robot_move_queue_.front();
        robot_move_queue_.pop_front();

        const auto robot = robot_positions_.find(command.robot_id);
        if (robot == robot_positions_.end()) {
            continue;
        }
        Position& position = robot->second;

        const Position next = position + command.direction;
        if (!contains(next.x, next.y)) {
            continue;
        }

        const std::size_t destination = index(next.x, next.y);
        if (snapshot[destination] || next_occupants[destination]) {
            continue;
        }

        const std::size_t origin = index(position.x, position.y);

        world_[origin].mark_departure(command.robot_id, trail_lifetime_);

        next_occupants[origin].reset();
        next_occupants[destination] = command.robot_id;
        position = next;
    }
}

template <int Width, int Height>
void Playground<Width, Height>::Cell::mark_departure(
    ObjectID robot_id, std::chrono::seconds lifetime
) {
    if (lifetime.count() <= 0) { return; }

    const auto now = Trail::Clock::now();
    auto [visit, inserted] = robot_visits.try_emplace(robot_id, now);
    if (!inserted) {
        visit->second.update(now);
    }
}

template <int Width, int Height>
void Playground<Width, Height>::Cell::clear_trail(ObjectID robot_id) {
    robot_visits.erase(robot_id);
}

template <int Width, int Height>
bool Playground<Width, Height>::commit_robot_tick(const std::vector<Occupant>& next_occupants) {
    const auto now = Trail::Clock::now();
    bool trail_changed = false;
    for (std::size_t i = 0; i < world_.size(); ++i) {
        Cell& cell = world_[i];
        cell.occupant = next_occupants[i];
        trail_changed |= cell.expire_trails(now, trail_lifetime_);
    }
    return trail_changed;
}

template <int Width, int Height>
const std::unordered_map<ObjectID, Trail>& Playground<Width, Height>::Cell::visits() const {
    return robot_visits;
}

template <int Width, int Height>
bool Playground<Width, Height>::Cell::expire_trails(
        Trail::Clock::time_point now,
        std::chrono::seconds lifetime
) {
    const int expired_visits = std::erase_if(robot_visits, [now, lifetime](const auto& visit) {
        return visit.second.expired(now, lifetime);
    });
    return expired_visits != 0;
}

template <int Width, int Height>
std::vector<ObjectID> Playground<Width, Height>::move_grid() {
    const std::lock_guard<std::mutex> lock(actions_mutex);

    lift_robots();
    rotate_grid();
    std::vector<ObjectID> failed_robots = place_robots_back();

    for (ObjectID robot_id: failed_robots) {
        remove_robot_locked(robot_id, /*clear_occupant=*/false);
    }
    return failed_robots;
}

template <int Width, int Height>
void Playground<Width, Height>::lift_robots() {
    for (const auto& [robot_id, position]: robot_positions_) {
        Cell& cell = world_[index(position.x, position.y)];
        cell.mark_departure(robot_id, trail_lifetime_);
        cell.occupant = std::nullopt;
    }
}

template <int Width, int Height>
void Playground<Width, Height>::rotate_grid() {
    top_row_ = (top_row_ + Height - 1) % Height;
    insert_new_row();
}

template <int Width, int Height>
void Playground<Width, Height>::insert_new_row() {
    std::mt19937 generator{std::random_device{}()};
    std::bernoulli_distribution place_barrier{barrier_probaility_};

    for (int x = 0; x < Width; ++x) {
        Cell& cell = world_[index(x, 0)];
        cell = Cell{};

        if (place_barrier(generator)) {
            ObjectID candidate_id = generate_object_id();
            if (candidate_id != -1) { cell.occupant = candidate_id; }
        }
    }
}

template <int Width, int Height>
std::vector<ObjectID> Playground<Width, Height>::place_robots_back() {
    std::vector<ObjectID> failed_robots;

    for (const auto& [robot_id, position]: robot_positions_) {
        Cell& cell = world_[index(position.x, position.y)];
        if (cell.occupant) {
            failed_robots.push_back(robot_id);
        } else {
            cell.occupant = robot_id;
        }
    }

    return failed_robots;
}

template <int Width, int Height>
PlaygroundSnapshot Playground<Width, Height>::snapshot() const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const auto now = Trail::Clock::now();
    PlaygroundSnapshot snapshot{Width, Height, {}};
    snapshot.cells.reserve(world_.size());
    for (int y = 0; y < Height; ++y) {
        for (int x = 0; x < Width; ++x) {
            const Cell& cell = world_[index(x, y)];
            CellSnapshot& copied_cell = snapshot.cells.emplace_back(CellSnapshot{cell.occupant, {}});
            if (cell.occupant) {
                copied_cell.occupant_type =
                    robot_positions_.contains(*cell.occupant) ? ObjectType::Robot : ObjectType::Barrier;
            }
            for (const auto& [robot_id, trail] : cell.visits()) {
                if (cell.occupant != robot_id && !trail.expired(now, trail_lifetime_)) {
                    copied_cell.trail_robot_ids.push_back(robot_id);
                }
            }
        }
    }
    return snapshot;
}

template <int Width, int Height>
bool Playground<Width, Height>::is_registered_robot(ObjectID robot_id) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    return robot_positions_.contains(robot_id);
}

template <int Width, int Height>
std::optional<Position> Playground<Width, Height>::robot_position(ObjectID robot_id) const {
    const std::lock_guard<std::mutex> lock(actions_mutex);
    const std::unordered_map<ObjectID, Position>::const_iterator robot =
        robot_positions_.find(robot_id);
    if (robot == robot_positions_.end()) {
        return std::nullopt;
    }
    return robot->second;
}

template <int Width, int Height>
int Playground<Width, Height>::robot_last_visited(ObjectID robot_id, const Position& position) const {
    if (!contains(position.x, position.y) || trail_lifetime_.count() <= 0) {
        return -1;
    }

    const std::lock_guard<std::mutex> lock(actions_mutex);
    const Cell& cell = world_[index(position.x, position.y)];
    if (cell.occupant == robot_id) {
        return 0;
    }
    const auto& visits = cell.visits();
    const auto visit = visits.find(robot_id);
    if (visit == visits.end()) {
        return -1;
    }

    return visit->second.age_seconds(Trail::Clock::now(), trail_lifetime_);
}

template <int Width, int Height>
bool Playground<Width, Height>::contains(int x, int y) const {
    return x >= 0 && x < Width && y >= 0 && y < Height;
}

template <int Width, int Height>
std::size_t Playground<Width, Height>::index(int x, int y) const {
    const std::size_t row = (top_row_ + static_cast<std::size_t>(y)) % Height;
    return row * Width + static_cast<std::size_t>(x);
}
