#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

struct MenuItem {
    std::string tag;
    std::string title;
    std::string command;
};

void runCommand(const std::string& cmd);
std::string getSystemStats(); 

// --- INTEGRACIÓN GPIO JOYSTICK ---
bool initJoystickGPIO();
int readJoystickInput(); // Retorna KEY_UP, KEY_DOWN, 10 (ENTER) o -1 si no hay pulso

#endif