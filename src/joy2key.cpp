#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <gpiod.hpp>
#include <thread>
#include <chrono>

void emit(int fd, int type, int code, int val) {
    struct input_event ie{};
    ie.type = type;
    ie.code = code;
    ie.value = val;
    write(fd, &ie, sizeof(ie));
}

int main() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "No se pudo abrir /dev/uinput. Ejecuta con sudo o ajusta permisos." << std::endl;
        return 1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);

    struct uinput_setup usetup{};
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "Abierto GPIO Keyboard");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    try {
        auto chip = gpiod::chip("/dev/gpiochip1");
        std::vector<gpiod::line::offset> offsets = {69, 70, 72, 73, 75};
        std::vector<int> keycodes = {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_ENTER};
        std::vector<bool> last_states(5, false);

        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::INPUT);
        settings.set_bias(gpiod::line::bias::PULL_UP);

        gpiod::line_config line_cfg;
        for (auto offset : offsets) {
            line_cfg.add_line_settings(offset, settings);
        }

        auto request = chip.prepare_request()
            .set_consumer("Joy2Key")
            .set_line_config(line_cfg)
            .do_request();

        std::cout << "Mapeador de Joystick a Teclado iniciado correctamente." << std::endl;

        while (true) {
            auto vals = request.get_values();

            for (size_t i = 0; i < vals.size(); ++i) {
                bool pressed = (vals[i] == gpiod::line::value::INACTIVE);
                if (pressed != last_states[i]) {
                    last_states[i] = pressed;
                    emit(fd, EV_KEY, keycodes[i], pressed ? 1 : 0);
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error GPIO: " << e.what() << std::endl;
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}