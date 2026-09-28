#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

struct MenuItem {
    std::string tag;
    std::string title;
    std::string command;
};

void runCommand(const std::string& cmd);
std::string getSystemStats(); // <-- Devuelve CPU, RAM y Temp

#endif