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

            for (size_t i = 0; i < vals.size(); ++i) {
                if (vals[i] == gpiod::line::value::INACTIVE) {
                    std::cout << "DETECTADO: " << names[i] << std::endl;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}