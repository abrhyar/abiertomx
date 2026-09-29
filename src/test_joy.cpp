#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <gpiod.hpp>

int main() {
    try {
        auto chip = gpiod::chip("/dev/gpiochip1");
        std::vector<gpiod::line::offset> offsets = {69, 70, 72, 73, 75};
        std::vector<std::string> names = {"ABAJO (PC5)", "ARRIBA (PC6)", "IZQUIERDA (PC8)", "DERECHA (PC9)", "CENTRO (PC11)"};

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::INPUT);
        settings.set_bias(gpiod::line::bias::PULL_UP);

        gpiod::line_config line_cfg;
        for (auto offset : offsets) {
            line_cfg.add_line_settings(offset, settings);
        }

        auto request = chip.prepare_request()
            .set_consumer("TestJoystick")
            .set_line_config(line_cfg)
            .do_request();

        std::cout << "--- PRUEBA DE JOYSTICK GPIO ---" << std::endl;
        std::cout << "Mueve la palanca o presiona los botones (Ctrl+C para salir)" << std::endl;

        while (true) {
    auto vals = request.get_values();

    std::cout << "\r[PC5/DWN:" << (vals[0] == gpiod::line::value::ACTIVE ? "1" : "0") << "] "
              << "[PC6/UP:"  << (vals[1] == gpiod::line::value::ACTIVE ? "1" : "0") << "] "
              << "[PC8/LFT:" << (vals[2] == gpiod::line::value::ACTIVE ? "1" : "0") << "] "
              << "[PC9/RHT:" << (vals[3] == gpiod::line::value::ACTIVE ? "1" : "0") << "] "
              << "[PC11/MID:"<< (vals[4] == gpiod::line::value::ACTIVE ? "1" : "0") << "]" 
              << std::flush;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}