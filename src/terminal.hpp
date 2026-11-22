#pragma once

#include <ncurses.h>

class Terminal {
    public:
        Terminal() {
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
        }
        ~Terminal() { endwin(); }
};
