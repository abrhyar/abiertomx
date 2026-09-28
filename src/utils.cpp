#include "utils.hpp"
#include <ncurses.h>
#include <cstdlib>
#include <fstream>

std::string getCPUTemp() {
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (!tempFile.is_open()) return "N/A";

    int rawTemp = 0;
    tempFile >> rawTemp;
    tempFile.close();

    int tempC = rawTemp / 1000;
    return std::to_string(tempC) + "C";
}

void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    endwin();
    std::system(cmd.c_str());
    refresh();
    curs_set(0);
}