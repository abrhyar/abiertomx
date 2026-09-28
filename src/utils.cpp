#include "utils.hpp"
#include <ncurses.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

std::string getSystemStats() {
    // 1. Lectura de Temperatura
    int tempC = 0;
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int rawTemp = 0;
        tempFile >> rawTemp;
        tempC = rawTemp / 1000;
        tempFile.close();
    }

    // 2. Lectura de RAM desde /proc/meminfo
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

    // 3. Lectura rápida de carga de CPU desde /proc/loadavg
    int cpuUsage = 0;
    std::ifstream loadFile("/proc/loadavg");
    if (loadFile.is_open()) {
        double load1 = 0;
        loadFile >> load1;
        loadFile.close();
        // Aproximación rápida para 4 núcleos (H616)
        cpuUsage = (int)((load1 / 4.0) * 100.0);
        if (cpuUsage > 100) cpuUsage = 100;
    }

    // Retorna todo formateado en una sola cadena compacta
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