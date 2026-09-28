#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <chrono>

#include "utils.hpp"
#include "games.hpp"
#include "camera.hpp"
#include "gallery.hpp"
#include "web.hpp"

int main() {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        assume_default_colors(COLOR_WHITE, COLOR_BLACK);

        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
    }

    std::vector<MenuItem> menuItems = {
        {"[PAD]", "Juegos", "SUBMENU_GAMES"},
        {"[WEB]", "Web", "SUBMENU_WEB"},
        {"[CAM]", "Camara", "SUBMENU_CAM"},
        {"[MUS]", "Musica", ""},
        {"[VID]", "Videos", "SUBMENU_VIDS"},
        {"[IMG]", "Fotos", "SUBMENU_PICS"},
        {"[CFG]", "Configuracion", "sudo armbian-config"},
        {"[OFF]", "Salir", "EXIT"}
    };

    int selected = 0;
    bool running = true;

    // Control de lectura de temperatura cada 30 segundos
    std::string currentTemp = getCPUTemp();
    auto lastTempCheck = std::chrono::steady_clock::now();

    while (running) {
        clear();

        // Actualizar temperatura solo si transcurrieron 30 segundos
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastTempCheck).count() >= 30) {
            currentTemp = getCPUTemp();
            lastTempCheck = now;
        }

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO v0.1 | ALk");
        attroff(COLOR_PAIR(2) | A_BOLD);

        std::string tempStr = "[" + currentTemp + "]";
        attron(COLOR_PAIR(3));
        mvprintw(0, 30, "%s", tempStr.c_str());
        attroff(COLOR_PAIR(3));

        mvhline(1, 0, ACS_HLINE, 38);

        int startY = 3;
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-5s %-26s", menuItems[i].tag.c_str(), menuItems[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-5s %s", menuItems[i].tag.c_str(), menuItems[i].title.c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar   [ENTER] Entrar");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)menuItems.size()) % (int)menuItems.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)menuItems.size();
                break;
            case 10:
            case KEY_ENTER:
                if (menuItems[selected].command == "EXIT") {
                    running = false;
                } else if (menuItems[selected].command == "SUBMENU_PICS") {
                    showImageGallery();
                } else if (menuItems[selected].command == "SUBMENU_VIDS") {
                    showVideoGallery();
                } else if (menuItems[selected].command == "SUBMENU_GAMES") {
                    showGamesMenu();
                } else if (menuItems[selected].command == "SUBMENU_CAM") {
                    showCameraView();
                } else if (menuItems[selected].command == "SUBMENU_WEB") {
                    showWebMenu();
                } else if (!menuItems[selected].command.empty()) {
                    runCommand(menuItems[selected].command);
                }
                break;
        }
    }

    endwin();
    return 0;
}