#include "games.hpp"
#include "utils.hpp"
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

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