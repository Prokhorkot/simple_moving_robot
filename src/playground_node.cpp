#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>

#include "moving_robot/msg/delete_robot.hpp"
#include "moving_robot/msg/grid_cell.hpp"
#include "moving_robot/msg/grid_snapshot.hpp"
#include "moving_robot/msg/move_command.hpp"
#include "moving_robot/srv/register_robot.hpp"
#include "rclcpp/rclcpp.hpp"

#include "playground.hpp"
#include "robot.hpp"

namespace {
    using DeleteRobot = moving_robot::msg::DeleteRobot;
    using GridCell = moving_robot::msg::GridCell;
    using GridSnapshot = moving_robot::msg::GridSnapshot;
    using MoveCommand = moving_robot::msg::MoveCommand;
    using RegisterRobot = moving_robot::srv::RegisterRobot;

    static_assert(static_cast<std::uint8_t>(ObjectType::Unknown) == GridCell::TYPE_UNKNOWN);
    static_assert(static_cast<std::uint8_t>(ObjectType::Robot) == GridCell::TYPE_ROBOT);
    static_assert(static_cast<std::uint8_t>(ObjectType::Barrier) == GridCell::TYPE_BARRIER);

    template <int Width, int Height>
    class PlaygroundNode : public rclcpp::Node {
        public:
            PlaygroundNode() : rclcpp::Node("playground") {
                using std::placeholders::_1;
                using std::placeholders::_2;

                register_service_ = create_service<RegisterRobot>(
                    "playground/register_robot",
                    std::bind(&PlaygroundNode::handle_register_robot, this, _1, _2));
                move_subscription_ = create_subscription<MoveCommand>(
                    "robot/move", 10,
                    std::bind(&PlaygroundNode::handle_move, this, _1));
                delete_subscription_ = create_subscription<DeleteRobot>(
                    "robot/delete", 10,
                    std::bind(&PlaygroundNode::handle_delete, this, _1));
                grid_publisher_ = create_publisher<GridSnapshot>("playground/grid", 10);
                round_ended_publisher_ = create_publisher<DeleteRobot>("/playground/round_ended", 10);
                tick_timer_ = create_wall_timer(
                    std::chrono::milliseconds{20},
                    std::bind(&PlaygroundNode::next_tick, this));
                grid_scroll_timer_ = create_wall_timer(
                    std::chrono::seconds{1},
                    std::bind(&PlaygroundNode::move_grid, this));
            }

        private:
            void next_tick() {
                if (playground_.next_tick()) {
                    publish_grid();
                }
            }

            void move_grid() {
                const std::vector<ObjectID> failed_robots = playground_.move_grid();
                if (!failed_robots.empty()) {
                    for (const ObjectID robot_id: failed_robots) {
                        DeleteRobot message;
                        message.robot_id = robot_id;
                        round_ended_publisher_->publish(message);
                    }
                }
                publish_grid();
            }

            void handle_delete(DeleteRobot::ConstSharedPtr message) {
                if (playground_.remove_robot(message->robot_id)) {
                    publish_grid();
                }
            }

            void publish_grid() {
                const PlaygroundSnapshot snapshot = playground_.snapshot();
                GridSnapshot message;
                message.width = snapshot.width;
                message.height = snapshot.height;
                message.cells.reserve(snapshot.cells.size());
                for (const CellSnapshot& cell : snapshot.cells) {
                    auto& output = message.cells.emplace_back();
                    output.occupant_id = cell.occupant.value_or(0);
                    output.occupant_type = static_cast<std::uint8_t>(cell.occupant_type);
                    output.trail_robot_ids.assign(
                        cell.trail_robot_ids.begin(), cell.trail_robot_ids.end());
                }
                grid_publisher_->publish(message);
            }

            void handle_move(MoveCommand::ConstSharedPtr command) {
                if (command->direction > static_cast<unsigned int>(Directions::Right)) {
                    return;
                }
                playground_.queue_robot_move(
                    command->robot_id, static_cast<Directions>(command->direction));
            }

            void handle_register_robot(
                const RegisterRobot::Request::SharedPtr request,
                RegisterRobot::Response::SharedPtr response
            ) {
                const Robot robot{Position{request->preferred_x, request->preferred_y}};
                const ObjectID id = playground_.register_robot(robot);
                response->success = id != -1;
                response->robot_id = id;
            }

            Playground<Width, Height> playground_{1, 0.05};
            rclcpp::Service<RegisterRobot>::SharedPtr register_service_;
            rclcpp::Subscription<MoveCommand>::SharedPtr move_subscription_;
            rclcpp::Subscription<DeleteRobot>::SharedPtr delete_subscription_;
            rclcpp::Publisher<GridSnapshot>::SharedPtr grid_publisher_;
            rclcpp::TimerBase::SharedPtr tick_timer_;
            rclcpp::Publisher<DeleteRobot>::SharedPtr round_ended_publisher_;
            rclcpp::TimerBase::SharedPtr grid_scroll_timer_;
    };

    using ConfiguredPlaygroundNode = PlaygroundNode<100, 100>;
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    int exit_code = 0;
    try {
        rclcpp::spin(std::make_shared<ConfiguredPlaygroundNode>());
    } catch (const std::exception&) {
        exit_code = 1;
    }
    rclcpp::shutdown();
    return exit_code;
}
