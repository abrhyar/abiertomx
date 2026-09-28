#include "web.hpp"
#include "utils.hpp"
#include <ncurses.h>
#include <vector>
#include <string>

void showWebMenu() {
    std::vector<MenuItem> webOptions = {
        {"[THO]", "Navegador Thorium", "THORIUM"},
        {"[YTV]", "YouTube TV", "YOUTUBE"}
    };

    int selected = 0;
    bool inWebMenu = true;

    while (inWebMenu) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Navegacion Web");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        int startY = 3;
        for (size_t i = 0; i < webOptions.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-5s %-26s", webOptions[i].tag.c_str(), webOptions[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-5s %s", webOptions[i].tag.c_str(), webOptions[i].title.c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Entrar  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)webOptions.size()) % (int)webOptions.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)webOptions.size();
                break;
            case 10:
            case KEY_ENTER:
                if (webOptions[selected].command == "THORIUM") {
                    std::string cmd = "sudo xinit /usr/bin/thorium-browser "
                                      "--kiosk --no-sandbox --ignore-gpu-blocklist "
                                      "--enable-gpu-rasterization --enable-zero-copy --use-gl=egl "
                                      "--enable-features=VaapiVideoDecoder,CanvasOopRasterization "
                                      "--alsa-output-device=default "
                                      "\"https://google.com\" -- :0";
                    runCommand(cmd);
                } else if (webOptions[selected].command == "YOUTUBE") {
                    std::string cmd = "sudo xinit /usr/bin/thorium-browser "
                                      "--kiosk --no-sandbox --ignore-gpu-blocklist "
                                      "--enable-gpu-rasterization --enable-zero-copy --use-gl=egl "
                                      "--enable-features=VaapiVideoDecoder,CanvasOopRasterization "
                                      "--alsa-output-device=default "
                                      "--user-agent=\"Mozilla/5.0 (SMART-TV; LINUX; Tizen 6.0) AppleWebKit/537.36 (KHTML, like Gecko) Version/6.0 TV Safari/537.36\" "
                                      "\"https://youtube.com/tv\" -- :0";
                    runCommand(cmd);
                }
                break;
            case 27:
            case 'q':
                inWebMenu = false;
                break;
        }
    }
}