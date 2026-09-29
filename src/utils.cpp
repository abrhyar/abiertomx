#include "utils.hpp"
#include <ncurses.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <gpiod.hpp>
#include <memory>

// Objeto global para mantener la petición GPIO abierta durante el programa
static std::unique_ptr<gpiod::line_request> gpio_request = nullptr;

bool initJoystickGPIO() {
    try {
        auto chip = gpiod::chip("/dev/gpiochip1");

        // Offsets del banco PC (69=DWN, 70=UP, 72=LFT, 73=RHT, 75=MID)
        std::vector<gpiod::line::offset> offsets = {69, 70, 72, 73, 75};

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::INPUT);
        settings.set_bias(gpiod::line::bias::PULL_UP);

        gpiod::line_config line_cfg;
        for (auto offset : offsets) {
            line_cfg.add_line_settings(offset, settings);
        }

        gpio_request = std::make_unique<gpiod::line_request>(
            chip.prepare_request()
                .set_consumer("Abierto_Joystick")
                .set_line_config(line_cfg)
                .do_request()
        );
        return true;
    } catch (...) {
        return false;
    }
}

int readJoystickInput() {
    if (!gpio_request) return -1;

    try {
        auto vals = gpio_request->get_values();

        // INACTIVE (0) significa que el pin se fue a masa/GND (pulsado)
        if (vals[1] == gpiod::line::value::INACTIVE) return KEY_UP;    // Linea 70 (UP)
        if (vals[0] == gpiod::line::value::INACTIVE) return KEY_DOWN;  // Linea 69 (DWN)
        if (vals[2] == gpiod::line::value::INACTIVE) return KEY_LEFT;  // Linea 72 (LFT)
        if (vals[3] == gpiod::line::value::INACTIVE) return KEY_RIGHT; // Linea 73 (RHT)
        if (vals[4] == gpiod::line::value::INACTIVE) return 10;        // Linea 75 (MID / Enter)
    } catch (...) {}

    return -1; // Sin pulsación
}

std::string getSystemStats() {
    int tempC = 0;
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int rawTemp = 0;
        tempFile >> rawTemp;
        tempC = rawTemp / 1000;
        tempFile.close();
    }

    int ramUsage = 0;
    std::ifstream memFile("/proc/meminfo");
    if (memFile.is_open()) {
        long totalMem = 0, freeMem = 0, buffers = 0, cached = 0;
        std::string key;
        long value;
        std::string unit;

        while (memFile >> key >> value >> unit) {
            if (key == "MemTotal:") totalMem = value;
            else if (key == "MemFree:") freeMem = value;
            else if (key == "Buffers:") buffers = value;
            else if (key == "Cached:") cached = value;
        }
        memFile.close();

        if (totalMem > 0) {
            long usedMem = totalMem - (freeMem + buffers + cached);
            ramUsage = (usedMem * 100) / totalMem;
        }
    }

    int cpuUsage = 0;
    std::ifstream loadFile("/proc/loadavg");
    if (loadFile.is_open()) {
        double load1 = 0;
        loadFile >> load1;
        loadFile.close();
        cpuUsage = (int)((load1 / 4.0) * 100.0);
        if (cpuUsage > 100) cpuUsage = 100;
    }

    std::ostringstream ss;
    ss << "C:" << cpuUsage << "% R:" << ramUsage << "% " << tempC << "C";
    return ss.str();
}

void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    endwin();
    std::system(cmd.c_str());
    refresh();
    curs_set(0);
}