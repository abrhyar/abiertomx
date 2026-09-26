#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>

struct MenuItem {
    std::string icon;
    std::string title;
};

int main() {
    // Permitir caracteres UTF-8 / Nerd Fonts / Emojis en NCurses
    setlocale(LC_ALL, "");

    initscr();              // Inicializar pantalla NCurses
    cbreak();               // Desactivar buffer de línea
    noecho();               // No mostrar teclas presionadas
    keypad(stdscr, TRUE);   // Habilitar flechas del teclado/mando
    curs_set(0);            // Ocultar cursor

    // Inicializar colores
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Resaltado de selección
        init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Header / Bordes
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Batería / Estado
    }

    std::vector<MenuItem> menuItems = {
        {"\uf11b", "Juegos (GBA, SNES)"},  // 🎮 Mando / Gamepad
        {"\uf001", "Música"},               // 🎵 Nota musical
        {"\uf03d", "Vídeos"},               // 🎬 Cámara de video
        {"\uf03e", "Fotos"},                // 🖼️ Imagen
        {"\uf013", "Configuración"},        // ⚙️ Engranaje
        {"\uf011", "Salir"}                 // ⚡ Botón Apagar
    };

    int selected = 0;
    bool running = true;

    while (running) {
        clear();

        // 1. Header (Rediseñado para pantalla de 2.8" / 320x240)
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO v0.1");
        mvprintw(0, 16, "| Launcher ALk");
        attroff(COLOR_PAIR(2) | A_BOLD);

        attron(COLOR_PAIR(3));
        mvprintw(0, 32, "🔋64%%");
        attroff(COLOR_PAIR(3));

        mvhline(1, 0, ACS_HLINE, 40);

        // 2. Lista Compacta del Menú ALk
        int startY = 3;
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %s %-28s", menuItems[i].icon.c_str(), menuItems[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%s %s", menuItems[i].icon.c_str(), menuItems[i].title.c_str());
            }
        }

        // 3. Footer de instrucciones en pantalla de 2.8"
        mvhline(12, 0, ACS_HLINE, 40);
        mvprintw(13, 1, "[↑↓] Navegar  [ENTER] Entrar");

        refresh();

        // Captura de entradas (Teclado o mapeo de Gamepad)
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + menuItems.size()) % menuItems.size();
                break;
            case KEY_DOWN:
                selected = (selected + 1) % menuItems.size();
                break;
            case 10: // Tecla ENTER
                if (selected == (int)menuItems.size() - 1) { // Opción Salir
                    running = false;
                }
                break;
            case 27: // Tecla ESC
                running = false;
                break;
        }
    }

    endwin(); // Cerrar NCurses limpiamente
    return 0;
}