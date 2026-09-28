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
#include <chrono>
#include <thread>
#include <regex>

namespace fs = std::filesystem;

struct LyricLine {
    double timestamp; // En segundos
    std::string text;
};

// Obtener la duración total de la canción en segundos mediante ffprobe
double getAudioDuration(const std::string& mp3Path) {
    std::string cmd = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + mp3Path + "\" > /tmp/duration.txt 2>/dev/null";
    std::system(cmd.c_str());
    std::ifstream file("/tmp/duration.txt");
    double duration = 0.0;
    if (file >> duration) {
        file.close();
        return duration;
    }
    file.close();
    return 180.0; // Valor por defecto (3 min) si no se detecta
}

// Extraer letras y detectar si vienen sincronizadas con marcas de tiempo [mm:ss.xx]
std::vector<LyricLine> getLyricsWithTimestamps(const std::string& mp3Path, double totalDuration, bool& isSynced) {
    std::string cmd = "ffprobe -v error -show_entries format_tags=lyrics:format_tags=USLT:format_tags=LYRICS:format_tags=description:format_tags=comment -of default=noprint_wrappers=1:nokey=1 \"" + mp3Path + "\" > /tmp/lyrics.txt 2>/dev/null";
    std::system(cmd.c_str());

    std::vector<LyricLine> result;
    std::ifstream file("/tmp/lyrics.txt");
    std::string line;
    std::vector<std::string> rawLines;

    std::regex timeRegex(R"(\[(\d{2}):(\d{2})(?:\.(\d{2,3}))?\]\s*(.*))");

    while (std::getline(file, line)) {
        if (line.empty() || line.find("http") != std::string::npos || line.find("Taken from") != std::string::npos) {
            continue;
        }
        rawLines.push_back(line);
    }
    file.close();

    isSynced = false;
    for (const auto& l : rawLines) {
        std::smatch match;
        if (std::regex_match(l, match, timeRegex)) {
            isSynced = true;
            double mins = std::stod(match[1].str());
            double secs = std::stod(match[2].str());
            double ms = match[3].matched ? std::stod(match[3].str()) / 100.0 : 0.0;
            double totalSecs = mins * 60.0 + secs + ms;
            result.push_back({totalSecs, match[4].str()});
        }
    }

    // Si no tiene tiempos [mm:ss], distribuir el texto uniformemente según la duración
    if (!isSynced && !rawLines.empty()) {
        double step = totalDuration / static_cast<double>(rawLines.size());
        for (size_t i = 0; i < rawLines.size(); ++i) {
            result.push_back({i * step, rawLines[i]});
        }
    }

    if (result.empty()) {
        result.push_back({0.0, "Sin letra disponible para esta pista."});
    }

    return result;
}

// Extraer portada limpia sin secuencias TrueColor de 24-bit
std::vector<std::string> getCoverArtANSI(const std::string& mp3Path, int width, int height) {
    std::string extractCmd = "ffmpeg -y -i \"" + mp3Path + "\" -an -vcodec copy /tmp/cover.jpg > /dev/null 2>&1";
    std::system(extractCmd.c_str());

    std::ostringstream chafaCmd;
    chafaCmd << "chafa --size=" << width << "x" << height << " --colors=16 --symbols=block /tmp/cover.jpg > /tmp/cover.txt 2>/dev/null";
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
    bool isSyncedLyrics = false;

    std::vector<LyricLine> currentLyrics;
    std::vector<std::string> currentCover;

    auto startTime = std::chrono::steady_clock::now();
    double songDuration = 0.0;

    // Hacer getch() no bloqueante para refrescar letras en tiempo real
    nodelay(stdscr, TRUE);

    while (inPlayer) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Reproductor de Musica");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 65);

        if (tracks.empty()) {
            mvprintw(3, 2, "No hay musica en ~/Music");
            mvhline(12, 0, ACS_HLINE, 65);
            mvprintw(13, 1, "[ESC/q] Volver");
            refresh();
            int ch = getch();
            if (ch == 27 || ch == 'q') inPlayer = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // 1. Cuadro de Portada (Arriba Izquierda)
        mvprintw(2, 1, "+--- PORTADA ---+");
        if (!currentCover.empty()) {
            for (size_t i = 0; i < currentCover.size() && i < 5; ++i) {
                mvprintw(3 + i, 2, "%.18s", currentCover[i].c_str());
            }
        } else {
            mvprintw(5, 4, "[ SIN PORTADA ]");
        }

        // 2. Cuadro de Lista de Pistas (Derecha)
        mvprintw(2, 22, "+--- PISTAS EN ~/Music ---+");
        int startY = 3;
        for (size_t i = 0; i < tracks.size() && i < 5; ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 22, "> %-35s", tracks[i].substr(0, 35).c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 24, "%-35s", tracks[i].substr(0, 35).c_str());
            }
        }

        // 3. Calculador de Letras y Desplazamiento Automático
        mvprintw(8, 1, "+--- LETRAS (AUTOSCROLL) ---+");
        
        size_t currentLineIdx = 0;
        if (isPlaying && !currentLyrics.empty()) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - startTime).count();

            for (size_t i = 0; i < currentLyrics.size(); ++i) {
                if (elapsed >= currentLyrics[i].timestamp) {
                    currentLineIdx = i;
                } else {
                    break;
                }
            }
        }

        // Mostrar 4 líneas a partir de la línea actual
        for (size_t i = 0; i < 4; ++i) {
            size_t targetIdx = currentLineIdx + i;
            if (targetIdx < currentLyrics.size()) {
                if (i == 0 && isPlaying) {
                    attron(A_BOLD | COLOR_PAIR(3));
                    mvprintw(9 + i, 2, "> %.58s", currentLyrics[targetIdx].text.c_str());
                    attroff(A_BOLD | COLOR_PAIR(3));
                } else {
                    mvprintw(9 + i, 4, "%.58s", currentLyrics[targetIdx].text.c_str());
                }
            }
        }

        // 4. Barra de Estado
        mvhline(13, 0, ACS_HLINE, 65);
        std::string statusStr = isPlaying ? "[PLAYING]" : "[PAUSED]";
        std::string modeStr = isSyncedLyrics ? "[LRC:SYNC]" : "[LRC:AUTO]";
        mvprintw(14, 1, "%s %s [ENT] Play  [S] Random  [ESC] Atras", statusStr.c_str(), modeStr.c_str());

        refresh();

        int ch = getch();
        if (ch != ERR) {
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
                    
                    songDuration = getAudioDuration(fullPath);
                    currentLyrics = getLyricsWithTimestamps(fullPath, songDuration, isSyncedLyrics);
                    currentCover = getCoverArtANSI(fullPath, 18, 5);

                    std::string playCmd = "killall mpv >/dev/null 2>&1; mpv --no-video --no-terminal \"" + fullPath + "\" &";
                    std::system(playCmd.c_str());

                    startTime = std::chrono::steady_clock::now();
                    isPlaying = true;
                    break;
                }
                case 27:
                case 'q':
                    inPlayer = false;
                    break;
            }
        }

        // Pequeña pausa para no saturar CPU en el bucle nodelay
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    // Restaurar bloqueo en getch al salir
    nodelay(stdscr, FALSE);
}