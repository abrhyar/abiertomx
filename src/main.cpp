#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <cstdlib>

struct MenuItem {
    std::string tag;
    std::string title;
    std::string command;
};

void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    // Pausar NCurses para dejar la TTY limpia
    endwin();
    std::system(cmd.c_str());
    
    // Restaurar NCurses al regresar
    refresh();
    curs_set(0);
}

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
        {"[PAD]", "Juegos (GBA, SNES)", ""},
        {"[MUS]", "Musica", ""},
        {"[VID]", "Videos", ""},
        // Abre todas las imágenes dentro de ~/Pictures de forma automática con auto-escala
        {"[IMG]", "Fotos", "fim -a ~/Pictures/"},
        {"[CFG]", "Configuracion", "sudo armbian-config"},
        {"[OFF]", "Salir", "EXIT"}
    };

    int selected = 0;
    bool running = true;

    while (running) {
        clear();

        // Header
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO v0.1 | ALk");
        attroff(COLOR_PAIR(2) | A_BOLD);

        attron(COLOR_PAIR(3));
        mvprintw(0, 30, "[64%%]");
        attroff(COLOR_PAIR(3));

        mvhline(1, 0, ACS_HLINE, 38);

        // Menú
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

        // Footer
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
                } else if (!menuItems[selected].command.empty()) {
                    runCommand(menuItems[selected].command);
                }
                break;
        }
    }

    endwin();
    return 0;
}