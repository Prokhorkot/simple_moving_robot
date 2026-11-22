cmake_minimum_required(VERSION 3.20)

project(moving_robot LANGUAGES C CXX)

set(MOVING_ROBOT_PLAYGROUND_WIDTH 100 CACHE STRING "Playground width in cells")
set(MOVING_ROBOT_PLAYGROUND_HEIGHT 100 CACHE STRING "Playground height in cells")
if(NOT MOVING_ROBOT_PLAYGROUND_WIDTH MATCHES "^[1-9][0-9]*$" OR
   NOT MOVING_ROBOT_PLAYGROUND_HEIGHT MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "Playground width and height must be positive integers")
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}_interfaces
    "msg/MoveCommand.msg"
    "msg/DeleteRobot.msg"
    "msg/GridCell.msg"
    "msg/GridSnapshot.msg"
    "srv/RegisterRobot.srv"
    LIBRARY_NAME ${PROJECT_NAME}
)
rosidl_get_typesupport_target(cpp_typesupport ${PROJECT_NAME}_interfaces rosidl_typesupport_cpp)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_library(playground_model STATIC
    src/position.cpp
    src/robot.cpp
    src/vision_info.cpp
)
target_include_directories(playground_model PUBLIC include PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_features(playground_model PUBLIC cxx_std_20)
target_compile_definitions(playground_model PUBLIC
    MOVING_ROBOT_PLAYGROUND_WIDTH=${MOVING_ROBOT_PLAYGROUND_WIDTH}
    MOVING_ROBOT_PLAYGROUND_HEIGHT=${MOVING_ROBOT_PLAYGROUND_HEIGHT}
)

add_executable(moving_robot src/main.cpp src/renderer.cpp)
target_include_directories(moving_robot PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(moving_robot PRIVATE playground_model rclcpp::rclcpp ${CURSES_LIBRARIES})

add_executable(playground_node src/playground_node.cpp)
target_link_libraries(playground_node PRIVATE
    playground_model rclcpp::rclcpp "${cpp_typesupport}"
)

add_executable(robot_node src/robot_node.cpp)
target_include_directories(robot_node PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(robot_node PRIVATE
    playground_model rclcpp::rclcpp "${cpp_typesupport}" ${CURSES_LIBRARIES}
)

add_executable(renderer_node src/renderer_node.cpp src/renderer.cpp)
target_include_directories(renderer_node PRIVATE include ${CURSES_INCLUDE_DIRS})
target_compile_features(renderer_node PRIVATE cxx_std_20)
target_link_libraries(renderer_node PRIVATE rclcpp::rclcpp "${cpp_typesupport}" ${CURSES_LIBRARIES})

install(TARGETS moving_robot playground_node robot_node renderer_node DESTINATION lib/${PROJECT_NAME})

ament_package()
