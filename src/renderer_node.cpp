#include <cstddef>
#include <exception>
#include <functional>
#include <memory>

#include "moving_robot/msg/grid_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

#include "renderer.hpp"
#include "terminal.hpp"

namespace {
    using GridSnapshot = moving_robot::msg::GridSnapshot;

    class RendererNode : public rclcpp::Node {
        public:
            RendererNode() : rclcpp::Node("renderer") {
                grid_subscription_ = create_subscription<GridSnapshot>(
                    "playground/grid", 10,
                    std::bind(&RendererNode::draw_snapshot, this, std::placeholders::_1));
            }

        private:
            static ObjectType occupant_type(ObjectID) {
                return ObjectType::Robot;
            }

            void draw_snapshot(GridSnapshot::ConstSharedPtr message) {
                if (message->width <= 0 || message->height <= 0) {
                    return;
                }
                const auto width = static_cast<std::size_t>(message->width);
                const auto height = static_cast<std::size_t>(message->height);
                if (message->cells.size() / width != height || message->cells.size() % width != 0) {
                    return;
                }

                PlaygroundSnapshot snapshot{message->width, message->height, {}};
                snapshot.cells.reserve(message->cells.size());
                for (const auto& cell : message->cells) {
                    auto& output = snapshot.cells.emplace_back();
                    if (cell.occupant_id != 0) {
                        output.occupant = cell.occupant_id;
                    }
                    output.trail_robot_ids.assign(
                        cell.trail_robot_ids.begin(), cell.trail_robot_ids.end());
                }
                renderer_.draw_frame(snapshot, &RendererNode::occupant_type, "[Ctrl+C to quit]");
            }

            Renderer renderer_;
            rclcpp::Subscription<GridSnapshot>::SharedPtr grid_subscription_;
    };
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    int exit_code = 0;
    try {
        const auto node = std::make_shared<RendererNode>();
        Terminal terminal;
        rclcpp::spin(node);
    } catch (const std::exception&) {
        exit_code = 1;
    }
    rclcpp::shutdown();
    return exit_code;
}
