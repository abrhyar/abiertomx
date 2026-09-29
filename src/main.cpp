#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include "utils.hpp"
#include "games.hpp"
#include "camera.hpp"
#include "gallery.hpp"
#include "web.hpp"
#include "music.hpp"

int main() {
    setlocale(LC_ALL, "");

    // Inicializar lectura GPIO
    initJoystickGPIO();

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Para que getch() no bloquee la lectura del joystick
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
        {"[MUS]", "Musica", "SUBMENU_MUS"},
        {"[VID]", "Videos", "SUBMENU_VIDS"},
        {"[IMG]", "Fotos", "SUBMENU_PICS"},
        {"[CFG]", "Configuracion", "sudo armbian-config"},
        {"[OFF]", "Salir", "EXIT"}
    };

    int selected = 0;
    bool running = true;

    std::string currentStats = getSystemStats();
    auto lastStatsCheck = std::chrono::steady_clock::now();

    while (running) {
        clear();

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsCheck).count() >= 10) {
            currentStats = getSystemStats();
            lastStatsCheck = now;
        }

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO");
        attroff(COLOR_PAIR(2) | A_BOLD);

        attron(COLOR_PAIR(3));
        mvprintw(0, 12, "[%s]", currentStats.c_str());
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

        // Captura dual: Teclado o Joystick GPIO
        int ch = getch();
        if (ch == ERR) {
            ch = readJoystickInput();
        }

        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)menuItems.size()) % (int)menuItems.size();
                std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Debounce
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)menuItems.size();
                std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Debounce
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
                } else if (menuItems[selected].command == "SUBMENU_MUS") {
                    showMusicPlayer();
                } else if (!menuItems[selected].command.empty()) {
                    runCommand(menuItems[selected].command);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    endwin();
    return 0;
}