#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>

#include <ncurses.h>

#include "moving_robot/msg/move_command.hpp"
#include "moving_robot/srv/register_robot.hpp"
#include "rclcpp/rclcpp.hpp"

#include "position.hpp"
#include "robot.hpp"
#include "terminal.hpp"

namespace {
    using MoveCommand = moving_robot::msg::MoveCommand;
    using RegisterRobot = moving_robot::srv::RegisterRobot;

    class RobotNode : public rclcpp::Node {
        public:
            explicit RobotNode(Position position = {0, 0})
                : rclcpp::Node("robot"),
                  robot_{position},
                  registration_client_{create_client<RegisterRobot>("/playground/register_robot")},
                  move_publisher_{create_publisher<MoveCommand>("robot/move", 10)} {
                if (!register_with_playground()) {
                    throw std::runtime_error("Robot registration failed");
                }
            }

            bool pollKeyboard() {
                const int key = getch();
                if (key == ERR || !rclcpp::ok()) {
                    return false;
                }
                const auto input = robot_.handle_input(key);
                if (input.direction) {
                    publishMove(*input.direction);
                }
                return input.keep_running;
            }

        private:
            Robot robot_;
            rclcpp::Client<RegisterRobot>::SharedPtr registration_client_;
            rclcpp::Publisher<MoveCommand>::SharedPtr move_publisher_;
            std::optional<std::int32_t> robot_id_;

            void publishMove(Directions direction) {
                MoveCommand command;
                command.robot_id = *robot_id_;
                command.direction = static_cast<std::uint8_t>(direction);
                move_publisher_->publish(command);
            }

            bool register_with_playground() {
                if (robot_id_) {
                    return true;
                }

                if (!registration_client_->wait_for_service()) {
                    return false;
                }
                if (!rclcpp::ok()) {
                    return false;
                }

                const Position spawn = robot_.preferred_spawn();
                const auto request = std::make_shared<RegisterRobot::Request>();
                request->preferred_x = spawn.x;
                request->preferred_y = spawn.y;

                rclcpp::Client<RegisterRobot>::FutureAndRequestId pending = registration_client_->async_send_request(request);
                const auto result = rclcpp::spin_until_future_complete(
                    get_node_base_interface(), pending, std::chrono::seconds{5});
                if (result != rclcpp::FutureReturnCode::SUCCESS) {
                    registration_client_->remove_pending_request(pending);
                    return false;
                }

                const auto response = pending.get();
                if (!response->success) {
                    return false;
                }

                robot_id_ = response->robot_id;
                return true;
            }
    };
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    int exit_code = 0;
    try {
        auto node = std::make_shared<RobotNode>();
        Terminal terminal;
        mvaddstr(0, 0, "Press Q to exit");
        refresh();
        while (rclcpp::ok() && node->pollKeyboard()) {
        }
    } catch (const std::exception&) {
        exit_code = 1;
    }
    rclcpp::shutdown();
    return exit_code;
}
