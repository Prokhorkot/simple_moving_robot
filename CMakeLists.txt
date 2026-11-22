cmake_minimum_required(VERSION 3.20)

project(moving_robot LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(moving_robot
    src/main.cpp
    src/position.cpp
    src/renderer.cpp
    src/robot.cpp
    src/vision_info.cpp
)

target_include_directories(moving_robot PRIVATE include ${CURSES_INCLUDE_DIRS})
target_link_libraries(moving_robot PRIVATE rclcpp::rclcpp ${CURSES_LIBRARIES})
target_compile_features(moving_robot PRIVATE cxx_std_20)

install(TARGETS moving_robot DESTINATION lib/${PROJECT_NAME})

ament_package()
