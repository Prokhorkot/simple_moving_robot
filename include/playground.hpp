#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "snapshots.hpp"
#include "position.hpp"
#include "trail.hpp"

class Robot;

template <int Width, int Height>
class Playground {
    static_assert(Width > 0 && Height > 0, "Playground dimensions must be positive");

    public:
        explicit Playground(int trail_lifetime_seconds = 5, double barrier_probaility = 0.1);

        ObjectID register_robot(const Robot& robot);
        bool remove_robot(ObjectID robot_id);

        bool is_registered_robot(ObjectID robot_id) const;
        std::optional<Position> robot_position(ObjectID robot_id) const;
        // Whole seconds since departure, 0 if occupied by this robot, or -1 if untracked.
        int robot_last_visited(ObjectID robot_id, const Position& position) const;

        bool queue_robot_move(ObjectID robot_id, Directions delta);
        bool next_tick();

        std::vector<ObjectID> move_grid();

        Position clip_to_bounds(Position position) const;
        bool is_free(int x, int y) const;
        PlaygroundSnapshot snapshot() const;

    private:
        using Occupant = std::optional<ObjectID>;

        struct Cell {
            Occupant occupant;

            void mark_departure(ObjectID robot_id, std::chrono::seconds lifetime);
            void clear_trail(ObjectID robot_id);
            bool expire_trails(Trail::Clock::time_point now, std::chrono::seconds lifetime);
            const std::unordered_map<ObjectID, Trail>& visits() const;

        private:
            std::unordered_map<ObjectID, Trail> robot_visits;
        };

        struct RobotMoveCommand {
            ObjectID robot_id;
            Directions direction;
        };

        void execute_robot_moves(
            const std::vector<Occupant>& snapshot,
            std::vector<Occupant>& next_occupants
        );
        bool commit_robot_tick(const std::vector<Occupant>& next_occupants);
        void lift_robots();
        void rotate_grid();
        void insert_new_row();
        std::vector<ObjectID> place_robots_back();

        // Caller must hold actions_mutex.
        bool remove_robot_locked(ObjectID robot_id, bool clear_occupant);

        bool contains(int x, int y) const;
        ObjectID generate_object_id();
        std::size_t index(int x, int y) const;

        mutable std::mutex actions_mutex;

        static constexpr std::size_t cell_count =
            static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height);
        std::chrono::seconds trail_lifetime_;
        std::array<Cell, cell_count> world_{};
        std::size_t top_row_ = 0;
        double barrier_probaility_;

        std::unordered_map<ObjectID, Position> robot_positions_;
        ObjectID last_object_id_ = 0;
        std::deque<RobotMoveCommand> robot_move_queue_;
};

#include "playground.tpp"
