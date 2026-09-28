#include "music.hpp"
#include "utils.hpp"
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<std::string> getLyrics(const std::string& mp3Path) {
    std::string cmd = "ffprobe -v error -show_entries format_tags=lyrics:format_tags=USLT:format_tags=LYRICS:format_tags=description:format_tags=comment -of default=noprint_wrappers=1:nokey=1 \"" + mp3Path + "\" > /tmp/lyrics.txt 2>/dev/null";
    std::system(cmd.c_str());

    std::vector<std::string> lines;
    std::ifstream file("/tmp/lyrics.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    file.close();

    if (lines.empty()) {
        lines.push_back("Sin letra encontrada.");
    }
    return lines;
}

std::vector<std::string> getCoverArtANSI(const std::string& mp3Path, int width, int height) {
    std::string extractCmd = "ffmpeg -y -i \"" + mp3Path + "\" -an -vcodec copy /tmp/cover.jpg > /dev/null 2>&1";
    std::system(extractCmd.c_str());

    std::ostringstream chafaCmd;
    chafaCmd << "chafa --size=" << width << "x" << height << " --symbols=block /tmp/cover.jpg > /tmp/cover.txt 2>/dev/null";
    std::system(chafaCmd.str().c_str());

    std::vector<std::string> ansiLines;
    std::ifstream file("/tmp/cover.txt");
    std::string line;
    while (std::getline(file, line)) {
        ansiLines.push_back(line);
    }
    file.close();

    return ansiLines;
}

void showMusicPlayer() {
    std::string folderPath = std::string(getenv("HOME")) + "/Music";
    std::vector<std::string> tracks;

    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".mp3" || ext == ".flac" || ext == ".ogg" || ext == ".wav") {
                    tracks.push_back(entry.path().filename().string());
                }
            }
        }
    }

    std::sort(tracks.begin(), tracks.end());

    int selected = 0;
    bool inPlayer = true;
    bool isPlaying = false;
    bool isShuffle = false;

    std::vector<std::string> currentLyrics;

    while (inPlayer) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Reproductor de Musica");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 60);

        if (tracks.empty()) {
            mvprintw(3, 2, "No hay musica en ~/Music");
            mvhline(12, 0, ACS_HLINE, 60);
            mvprintw(13, 1, "[ESC/q] Volver");
            refresh();
            int ch = getch();
            if (ch == 27 || ch == 'q') inPlayer = false;
            continue;
        }

        // 1. Cuadro de Portada
        mvprintw(2, 1, "+--- PORTADA ---+");
        
        // 2. Cuadro de Lista de Pistas
        mvprintw(2, 22, "+--- PISTAS EN ~/Music ---+");
        int startY = 3;
        for (size_t i = 0; i < tracks.size() && i < 6; ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 22, "> %-30s", tracks[i].substr(0, 30).c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 24, "%-30s", tracks[i].substr(0, 30).c_str());
            }
        }

        // 3. Cuadro de Letras
        mvprintw(9, 1, "+--- LETRAS ---+");
        for (size_t i = 0; i < currentLyrics.size() && i < 3; ++i) {
            mvprintw(10 + i, 2, "%.18s", currentLyrics[i].c_str());
        }

        // 4. Barra de Estado
        mvhline(13, 0, ACS_HLINE, 60);
        std::string statusStr = isPlaying ? "[PLAYING]" : "[PAUSED]";
        std::string shufStr = isShuffle ? "[SHUF:ON]" : "[SHUF:OFF]";
        mvprintw(14, 1, "%s  %s  [ENT] Play  [S] Aleatorio  [ESC] Salir", statusStr.c_str(), shufStr.c_str());

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)tracks.size()) % (int)tracks.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)tracks.size();
                break;
            case 's':
            case 'S':
                isShuffle = !isShuffle;
                break;
            case 10:
            case KEY_ENTER: {
                std::string fullPath = folderPath + "/" + tracks[selected];
                
                // Extraer letras
                currentLyrics = getLyrics(fullPath);

                // Dibujar portada con Chafa
                std::vector<std::string> cover = getCoverArtANSI(fullPath, 18, 5);
                for (size_t i = 0; i < cover.size() && i < 5; ++i) {
                    mvprintw(3 + i, 2, "%s", cover[i].c_str());
                }

                // Reproducir de fondo
                std::string playCmd = "killall mpv >/dev/null 2>&1; mpv --no-video --no-terminal \"" + fullPath + "\" &";
                std::system(playCmd.c_str());
                isPlaying = true;
                break;
            }
            case 27:
            case 'q':
                inPlayer = false;
                break;
        }
    }
}