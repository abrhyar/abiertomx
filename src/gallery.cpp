#include "gallery.hpp"
#include "utils.hpp"
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

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