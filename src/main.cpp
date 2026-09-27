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

// --- SUBMENÚ DE FOTOS ---
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

// --- SUBMENÚ DE VIDEOS ---
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

// --- EXPLORADOR DE ROMS GENÉRICO ---
void showRomExplorer(const std::string& systemName, const std::string& folderName, const std::vector<std::string>& extensions, const std::string& corePath) {
    std::string folderPath = std::string(getenv("HOME")) + "/ROMs/" + folderName;
    std::vector<std::string> roms;

    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                for (const auto& validExt : extensions) {
                    if (ext == validExt) {
                        roms.push_back(entry.path().filename().string());
                        break;
                    }
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
        mvprintw(0, 1, ("ABIERTO | " + systemName).c_str());
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        if (roms.empty()) {
            mvprintw(3, 2, ("No hay juegos en ~/ROMs/" + folderName).c_str());
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
                std::string cmd;
                if (!corePath.empty()) {
                    cmd = "retroarch -L " + corePath + " -f \"" + fullPath + "\"";
                } else {
                    // Para ejecutables de juegos variados/nativos
                    cmd = "\"" + fullPath + "\"";
                }
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

// --- SUBMENÚ PRINCIPAL DE JUEGOS ---
void showGamesMenu() {
    std::vector<MenuItem> gameSystems = {
        {"[GBA]", "Game Boy Advance", "GBA"},
        {"[NES]", "Nintendo NES", "NES"},
        {"[VAR]", "Juegos Variados", "MISC"}
    };

    int selected = 0;
    bool inSubmenu = true;

    while (inSubmenu) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Selector de Juegos");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        int startY = 3;
        for (size_t i = 0; i < gameSystems.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-5s %-26s", gameSystems[i].tag.c_str(), gameSystems[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-5s %s", gameSystems[i].tag.c_str(), gameSystems[i].title.c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Entrar  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)gameSystems.size()) % (int)gameSystems.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)gameSystems.size();
                break;
            case 10:
            case KEY_ENTER:
                if (gameSystems[selected].command == "GBA") {
                    showRomExplorer("Game Boy Advance", "GBA", {".gba", ".zip"}, "/usr/lib/aarch64-linux-gnu/libretro/mgba_libretro.so");
                } else if (gameSystems[selected].command == "NES") {
                    showRomExplorer("Nintendo (NES)", "NES", {".nes", ".zip"}, "/usr/lib/aarch64-linux-gnu/libretro/nestopia_libretro.so");
                } else if (gameSystems[selected].command == "MISC") {
                    showRomExplorer("Juegos Variados", "Misc", {".sh", ".elf", ".bin"}, "");
                }
                break;
            case 27:
            case 'q':
                inSubmenu = false;
                break;
        }
    }
}

// --- MENÚ PRINCIPAL ---
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
                } else if (menuItems[selected].command == "SUBMENU_GAMES") {
                    showGamesMenu();
                } else if (!menuItems[selected].command.empty()) {
                    runCommand(menuItems[selected].command);
                }
                break;
        }
    }

    endwin();
    return 0;
}