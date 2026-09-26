#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>

struct MenuItem {
    std::string tag;
    std::string title;
};

int main() {
    // Permitir soporte UTF-8 en NCurses
    setlocale(LC_ALL, "");

    initscr();              // Inicializar NCurses
    cbreak();               // Desactivar buffer de línea
    noecho();               // Ocultar teclas presionadas
    keypad(stdscr, TRUE);   // Habilitar flechas del teclado/mando
    curs_set(0);            // Ocultar cursor

    // Inicializar colores y forzar fondo negro
    if (has_colors()) {
        start_color();
        use_default_colors();
        assume_default_colors(COLOR_WHITE, COLOR_BLACK);

        init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Resaltado de selección
        init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Header / Bordes
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Batería / Estado
    }

    std::vector<MenuItem> menuItems = {
        {"🎮", "Juegos (GBA, SNES)"},
        {"🎵", "Música"},
        {"🎬", "Vídeos"},
        {"🖼️ ", "Fotos"},
        {"⚙️ ", "Configuración"},
        {"⚡", "Salir"}
    };

    int selected = 0;
    bool running = true;

    while (running) {
        clear();

        // 1. Header (Ajustado a 320x240)
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO v0.1");
        mvprintw(0, 15, "| Launcher ALk");
        attroff(COLOR_PAIR(2) | A_BOLD);

        attron(COLOR_PAIR(3));
        mvprintw(0, 32, "[64%%]");
        attroff(COLOR_PAIR(3));

        mvhline(1, 0, ACS_HLINE, 40);

        // 2. Lista de Opciones
        int startY = 3;
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-5s %-28s", menuItems[i].tag.c_str(), menuItems[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-5s %s", menuItems[i].tag.c_str(), menuItems[i].title.c_str());
            }
        }

        // 3. Footer de Controles
        mvhline(12, 0, ACS_HLINE, 40);
        mvprintw(13, 1, "[^v] Navegar   [ENTER] Entrar");

        refresh();

        // Lectura de teclas
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + menuItems.size()) % menuItems.size();
                break;
            case KEY_DOWN:
                selected = (selected + 1) % menuItems.size();
                break;
            case 10: // Enter
                if (selected == (int)menuItems.size() - 1) {
                    running = false;
                }
                break;
            case 27: // Esc
                running = false;
                break;
        }
    }

    endwin();
    return 0;
}