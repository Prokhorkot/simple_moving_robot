#include <ncurses.h>

#include "robot.hpp"
#include "playground.hpp"


int main() {
    moving_robot::Playground playground{10, 10};
    moving_robot::Robot robot;
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    mvprintw(0, 0, "Robot position: (%2d, %2d)  [q to quit]", 
        robot.get_coords().x, robot.get_coords().y);
    refresh();

    while(true) {
        moving_robot::Position position = robot.get_coords();

        mvprintw(0, 17, "%2d", position.x);
        mvprintw(0, 21, "%2d", position.y);
        refresh();

        const int key = getch();
        if (key == 'q' || key == '\x1b' || key == ERR) {
            break;
        }

        switch (key)
        {
            case KEY_UP: position = robot.try_move(moving_robot::Directions::Up, playground); break;
            case KEY_LEFT: position = robot.try_move(moving_robot::Directions::Left, playground); break;
            case KEY_DOWN: position = robot.try_move(moving_robot::Directions::Down, playground); break;
            case KEY_RIGHT: position = robot.try_move(moving_robot::Directions::Right, playground); break;
        }
    }
    endwin();
    return 0;
}