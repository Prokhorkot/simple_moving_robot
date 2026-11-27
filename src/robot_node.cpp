#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>

#include <ncurses.h>

#include "moving_robot/msg/delete_robot.hpp"
#include "moving_robot/msg/move_command.hpp"
#include "moving_robot/srv/register_robot.hpp"
#include "rclcpp/rclcpp.hpp"

#include "position.hpp"
#include "robot.hpp"
#include "terminal.hpp"

namespace {
    using DeleteRobot = moving_robot::msg::DeleteRobot;
    using MoveCommand = moving_robot::msg::MoveCommand;
    using RegisterRobot = moving_robot::srv::RegisterRobot;

    class RobotNode : public rclcpp::Node {
        public:
            explicit RobotNode(Position position = {0, 0})
                : rclcpp::Node("robot"),
                  robot_{position},
                  registration_client_{create_client<RegisterRobot>("/playground/register_robot")},
                  move_publisher_{create_publisher<MoveCommand>("robot/move", 10)},
                  delete_publisher_{create_publisher<DeleteRobot>("robot/delete", 10)} {
                round_ended_subscription_ = create_subscription<DeleteRobot>(
                    "/playground/round_ended", 10,
                    std::bind(&RobotNode::handle_round_ended, this, std::placeholders::_1));
                if (!register_with_playground()) {
                    throw std::runtime_error("Robot registration failed");
                }
            }

            ~RobotNode() override {
                request_removal();
            }

            bool round_ended() const {
                return round_ended_.load();
            }

            bool pollKeyboard() {
                const int key = getch();
                if (!rclcpp::ok() || round_ended()) {
                    return false;
                }
                if (key == ERR) {
                    return true;
                }
                const auto input = robot_.handle_input(key);
                if (!input.keep_running) {
                    request_removal();
                    return false;
                }
                if (input.direction) {
                    MoveCommand command;
                    command.robot_id = *robot_id_;
                    command.direction = static_cast<std::uint8_t>(*input.direction);
                    move_publisher_->publish(command);
                }
                return true;
            }

        private:
            Robot robot_;
            rclcpp::Client<RegisterRobot>::SharedPtr registration_client_;
            rclcpp::Publisher<MoveCommand>::SharedPtr move_publisher_;
            rclcpp::Publisher<DeleteRobot>::SharedPtr delete_publisher_;
            rclcpp::Subscription<DeleteRobot>::SharedPtr round_ended_subscription_;
            std::optional<std::int32_t> robot_id_;
            std::atomic_bool round_ended_{false};

            void request_removal() {
                if (!robot_id_ || round_ended() || !rclcpp::ok()) {
                    return;
                }
                try {
                    DeleteRobot message;
                    message.robot_id = *robot_id_;
                    delete_publisher_->publish(message);
                    delete_publisher_->wait_for_all_acked(std::chrono::milliseconds{200});
                } catch (const std::exception&) {
                }
            }

            void handle_round_ended(DeleteRobot::ConstSharedPtr message) {
                if (robot_id_ == message->robot_id) {
                    round_ended_.store(true);
                }
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
    bool round_ended = false;
    try {
        auto node = std::make_shared<RobotNode>();
        Terminal terminal;
        timeout(50);
        mvaddstr(0, 0, "Press Q to exit");
        refresh();

        bool spin_failed = false;
        std::thread ros_thread([node, &spin_failed] {
            try {
                rclcpp::spin(node);
            } catch (const std::exception&) {
                if (rclcpp::ok()) {
                    spin_failed = true;
                    rclcpp::shutdown();
                }
            }
        });

        try {
            while (rclcpp::ok() && !node->round_ended() && node->pollKeyboard()) {
            }
        } catch (const std::exception&) {
            exit_code = 1;
        }
        rclcpp::shutdown();
        ros_thread.join();
        round_ended = node->round_ended();
        if (spin_failed) {
            exit_code = 1;
        }
    } catch (const std::exception&) {
        exit_code = 1;
    }
    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }
    if (round_ended) {
        std::puts("Round over.");
    }
    return exit_code;
}
