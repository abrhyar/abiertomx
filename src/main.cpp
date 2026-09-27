#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

struct MenuItem {
    std::string tag;
    std::string title;
    std::string command;
};

void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    
    endwin();
    std::system(cmd.c_str());
    refresh();
    curs_set(0);
}

// Submenú para Imágenes
void showImageGallery() {
    std::string folderPath = std::string(getenv("HOME")) + "/Pictures";
    std::vector<std::string> images;

    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                    images.push_back(entry.path().filename().string());
                }
            }
        }
    }

    std::sort(images.begin(), images.end());

    int selected = 0;
    bool inGallery = true;

    while (inGallery) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Galeria de Fotos");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        if (images.empty()) {
            mvprintw(3, 2, "No hay imagenes en ~/Pictures");
            mvhline(12, 0, ACS_HLINE, 38);
            mvprintw(13, 1, "[ESC/q] Volver");
            refresh();

            int ch = getch();
            if (ch == 27 || ch == 'q') inGallery = false;
            continue;
        }

        int startY = 3;
        for (size_t i = 0; i < images.size() && i < 8; ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-32s", images[i].substr(0, 32).c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-32s", images[i].substr(0, 32).c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Ver  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)images.size()) % (int)images.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)images.size();
                break;
            case 10:
            case KEY_ENTER: {
                std::string fullPath = folderPath + "/" + images[selected];
                std::string cmd = "fim -a \"" + fullPath + "\"";
                runCommand(cmd);
                break;
            }
            case 27:
            case 'q':
                inGallery = false;
                break;
        }
    }
}

// Submenú para Videos
void showVideoGallery() {
    std::string folderPath = std::string(getenv("HOME")) + "/Videos";
    std::vector<std::string> videos;

    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    videos.push_back(entry.path().filename().string());
                }
            }
        }
    }

    std::sort(videos.begin(), videos.end());

    int selected = 0;
    bool inGallery = true;

    while (inGallery) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Reproductor de Video");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        if (videos.empty()) {
            mvprintw(3, 2, "No hay videos en ~/Videos");
            mvhline(12, 0, ACS_HLINE, 38);
            mvprintw(13, 1, "[ESC/q] Volver");
            refresh();

            int ch = getch();
            if (ch == 27 || ch == 'q') inGallery = false;
            continue;
        }

        int startY = 3;
        for (size_t i = 0; i < videos.size() && i < 8; ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-32s", videos[i].substr(0, 32).c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-32s", videos[i].substr(0, 32).c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Ver  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)videos.size()) % (int)videos.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)videos.size();
                break;
            case 10:
            case KEY_ENTER: {
                std::string fullPath = folderPath + "/" + videos[selected];
                std::string cmd = "mpv --vo=gpu,drm,tesseract --fs \"" + fullPath + "\"";
                runCommand(cmd);
                break;
            }
            case 27:
            case 'q':
                inGallery = false;
                break;
        }
    }
}

// Submenú para Juegos GBA
void showGbaGallery() {
    std::string folderPath = std::string(getenv("HOME")) + "/ROMs/GBA";
    std::vector<std::string> roms;

    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".gba" || ext == ".zip") {
                    roms.push_back(entry.path().filename().string());
                }
            }
        }
    }

    std::sort(roms.begin(), roms.end());

    int selected = 0;
    bool inGallery = true;

    while (inGallery) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Game Boy Advance");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        if (roms.empty()) {
            mvprintw(3, 2, "No hay ROMs en ~/ROMs/GBA");
            mvhline(12, 0, ACS_HLINE, 38);
            mvprintw(13, 1, "[ESC/q] Volver");
            refresh();

            int ch = getch();
            if (ch == 27 || ch == 'q') inGallery = false;
            continue;
        }

        int startY = 3;
        for (size_t i = 0; i < roms.size() && i < 8; ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-32s", roms[i].substr(0, 32).c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-32s", roms[i].substr(0, 32).c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Jugar  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)roms.size()) % (int)roms.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)roms.size();
                break;
            case 10:
            case KEY_ENTER: {
                std::string fullPath = folderPath + "/" + roms[selected];
                // Lanza X11 rápido solo para mGBA a pantalla completa
                std::string cmd = "xinit mgba-qt -f \"" + fullPath + "\" -- :0";
                runCommand(cmd);
                break;
            }
            case 27:
            case 'q':
                inGallery = false;
                break;
        }
    }
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
        {"[GBA]", "Juegos GBA", "SUBMENU_GBA"},
        {"[MUS]", "Musica", ""},
        {"[VID]", "Videos", "SUBMENU_VIDS"},
        {"[IMG]", "Fotos", "SUBMENU_PICS"},
        {"[CFG]", "Configuracion", "sudo armbian-config"},
        {"[OFF]", "Salir", "EXIT"}
    };

    int selected = 0;
    bool running = true;

    while (running) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO v0.1 | ALk");
        attroff(COLOR_PAIR(2) | A_BOLD);

        attron(COLOR_PAIR(3));
        mvprintw(0, 30, "[64%%]");
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
                } else if (menuItems[selected].command == "SUBMENU_GBA") {
                    showGbaGallery();
                } else if (!menuItems[selected].command.empty()) {
                    runCommand(menuItems[selected].command);
                }
                break;
        }
    }

    endwin();
    return 0;
}