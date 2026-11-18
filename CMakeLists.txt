cmake_minimum_required(VERSION 3.16)

project(moving_robot LANGUAGES CXX)

add_executable(moving_robot
    src/main.cpp
    src/playground.cpp
    src/robot.cpp
)


set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

target_include_directories(moving_robot PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(moving_robot PRIVATE ${CURSES_LIBRARIES})

target_include_directories(moving_robot PRIVATE include)
target_compile_features(moving_robot PRIVATE cxx_std_20)