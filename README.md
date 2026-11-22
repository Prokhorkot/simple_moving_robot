# Simple Moving Robot

A C++20 terminal simulation of a robot moving around a grid. Use the arrow keys to move the robot, and watch its position and trail in a separate terminal. The `moving_robot` package uses ROS 2 for communication and ncurses for terminal input and rendering.

## Nodes

| Executable | Role |
| --- | --- |
| `playground_node` | Owns the grid, registers robots, and processes queued moves every 20 ms. It publishes a grid snapshot when it processes a nonempty command queue. |
| `robot_node` | Registers with the playground, waits for arrow keys, and publishes movement commands with its assigned robot ID. |
| `renderer_node` | Subscribes to grid snapshots and redraws the terminal when a snapshot arrives. |

Registration uses the `/playground/register_robot` service. Movement commands use `/robot/move`, and grid snapshots use `/playground/grid`.

## Build

The project was tested on Ubuntu 26.04 with ROS 2 Lyrical. You'll need a C++20 compiler, CMake, `colcon`, the ROS 2 C++ and interface-generation tools, and the ncurses development headers (`libncurses-dev` on Ubuntu).

From the repository root:

```bash
source /opt/ros/lyrical/setup.bash
colcon build --paths . --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3
```

Use the corresponding `setup.zsh` script instead if you use Zsh. The explicit Python path keeps ROS interface generation on the system interpreter when a different Python is active.

## Run

From the repository root, source `install/setup.bash` in each of three terminals, then run one command per terminal:

```bash
ros2 run moving_robot playground_node
ros2 run moving_robot renderer_node
ros2 run moving_robot robot_node
```

Focus the `robot_node` terminal and press an arrow key. The renderer draws its first frame when it receives the first grid update. The robot is shown as `R` and its trail as `.`. Press `Q` or Escape to quit `robot_node`, and `Ctrl+C` to stop the other nodes.

You can also run the standalone terminal version with `ros2 run moving_robot moving_robot`.

## License

[Apache License 2.0](LICENSE).
