#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "snapshots.hpp"
#include "position.hpp"
#include "vision_info.hpp"

class Robot;

template <int Width, int Height>
class Playground {
    static_assert(Width > 0 && Height > 0, "Playground dimensions must be positive");

    public:
        explicit Playground(int max_traceback = 5);
        Position clip_to_bounds(Position position) const;
        ObjectID register_robot(const Robot& robot);
        bool remove_robot(ObjectID robot_id);
        bool queue_robot_move(ObjectID robot_id, Directions delta);
        bool is_free(int x, int y) const;
        bool next_tick();
        PlaygroundSnapshot snapshot() const;
        bool is_registered_robot(ObjectID robot_id) const;
        std::optional<Position> robot_position(ObjectID robot_id) const;
        int robot_last_visited(ObjectID robot_id, const Position& position) const;

    private:
        mutable std::mutex actions_mutex;

        using Occupant = std::optional<ObjectID>;

        struct Cell {
            Occupant occupant;
            std::unordered_map<ObjectID, VisionInfo> robot_visits;
        };

        struct RobotMoveCommand {
            ObjectID robot_id;
            Directions direction;
        };

        void execute_robot_moves(
            const std::vector<Occupant>& snapshot,
            std::vector<Occupant>& next_occupants
        );
        bool update_grid(const std::vector<Occupant>& next_occupants);
        bool contains(int x, int y) const;
        bool is_free_unlocked(int x, int y) const;
        std::size_t index(int x, int y) const;

        static constexpr std::size_t cell_count =
            static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height);
        int max_traceback_;
        std::array<Cell, cell_count> world_{};
        std::unordered_map<ObjectID, Position> robot_positions_;
        ObjectID last_robot_id_ = 0;
        std::deque<RobotMoveCommand> robot_move_queue_;
};

#include "playground.tpp"
