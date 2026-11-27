#pragma once

#include "snapshots.hpp"
#include <ncurses.h>

class Renderer {
    public:
        void draw_frame(
            const PlaygroundSnapshot& snapshot,
            const char* status = "[q to quit]"
        ) const;

    private:
        static chtype symbol_for(ObjectType type);
};
