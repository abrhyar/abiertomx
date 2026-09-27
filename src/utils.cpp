#include "utils.hpp"
#include <ncurses.h>
#include <cstdlib>

void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    endwin();
    std::system(cmd.c_str());
    refresh();
    curs_set(0);
}