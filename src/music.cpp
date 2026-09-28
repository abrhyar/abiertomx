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

// Obtener la duración de la pista con ffprobe
double getAudioDuration(const std::string& mp3Path) {
    std::string cmd = "ffprobe -v error -show_entries format:duration -of default=noprint_wrappers=1:nokey=1 \"" + mp3Path + "\" > /tmp/duration.txt 2>/dev/null";
    std::system(cmd.c_str());
    std::ifstream file("/tmp/duration.txt");
    double duration = 0.0;
    if (file >> duration) {
        file.close();
        return duration;
    }
    file.close();
    return 180.0;
}

// Extraer letras y detectar si tienen marcas de tiempo [mm:ss.xx]
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

// Pantalla 1: Selector previo de Temporizador (Controlable con D-Pad/A)
int selectTimerMenu() {
    std::vector<std::string> options = {"No (Desactivado)", "30 Minutos", "1 Hora", "1 Hora 30 Minutos"};
    int selected = 0;
    bool selecting = true;

    while (selecting) {
        erase();
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, 2, "=== AJUSTE DE TEMPORIZADOR ===");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvprintw(3, 2, "¿Deseas apagar la musica en?");

        for (size_t i = 0; i < options.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(5 + i, 4, "> [ %s ]", options[i].c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(5 + i, 6, "  %s  ", options[i].c_str());
            }
        }

        mvhline(11, 0, ACS_HLINE, 50);
        mvprintw(12, 2, "(Arriba/Abajo: Mover | A/Enter: Confirmar)");
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + options.size()) % options.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % options.size();
                break;
            case 10:
            case KEY_ENTER:
            case 'a':
            case 'A':
                selecting = false;
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    // Retorna minutos elegidos
    if (selected == 1) return 30;
    if (selected == 2) return 60;
    if (selected == 3) return 90;
    return 0;
}

void showMusicPlayer() {
    // 1. Selector de temporizador al abrir la app de música
    int timerMinutes = selectTimerMenu();
    auto timerStartTime = std::chrono::steady_clock::now();

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
    bool isSyncedLyrics = false;

    std::vector<LyricLine> currentLyrics;
    auto startTime = std::chrono::steady_clock::now();
    double songDuration = 0.0;

    nodelay(stdscr, TRUE);

    while (inPlayer) {
        erase();

        // Verificación del Temporizador (Apaga mpv al vencer el tiempo)
        if (timerMinutes > 0 && isPlaying) {
            auto nowTimer = std::chrono::steady_clock::now();
            double elapsedMin = std::chrono::duration<double, std::ratio<60>>(nowTimer - timerStartTime).count();
            if (elapsedMin >= timerMinutes) {
                std::system("killall mpv >/dev/null 2>&1");
                isPlaying = false;
                timerMinutes = 0;
            }
        }

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Reproductor (Shuffle On)");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 60);

        // 2. Lista de Opciones navegables (Pistas + Controles + Salir)
        // Estructura de navegación:
        // Index 0: Pista 1
        // Index 1: Pista 2...
        // Index N: [ CERRAR Y PAUSAR MUSICA ]
        
        int totalItems = tracks.size() + 1; // +1 para la opción de Salir

        mvprintw(2, 1, "+--- PISTAS Y ACCIONES ---+");
        int startY = 3;

        for (int i = 0; i < totalItems && i < 6; ++i) {
            if (i < (int)tracks.size()) {
                // Dibujar Pistas de audio
                if (i == selected) {
                    attron(COLOR_PAIR(1) | A_BOLD);
                    mvprintw(startY + i, 1, "> %-50s", tracks[i].substr(0, 50).c_str());
                    attroff(COLOR_PAIR(1) | A_BOLD);
                } else {
                    mvprintw(startY + i, 3, "%-50s", tracks[i].substr(0, 50).c_str());
                }
            } else {
                // Opción Seleccionable para Cerrar y liberar recursos
                if (i == selected) {
                    attron(COLOR_PAIR(3) | A_BOLD);
                    mvprintw(startY + i, 1, "> [ X ] DETENER Y CERRAR APLICACION");
                    attroff(COLOR_PAIR(3) | A_BOLD);
                } else {
                    mvprintw(startY + i, 3, "[ X ] DETENER Y CERRAR APLICACION");
                }
            }
        }

        // 3. Cuadro de Letras (Autoscroll)
        mvprintw(10, 1, "+--- LETRAS EN TIEMPO REAL ---+");
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

        for (size_t i = 0; i < 2; ++i) {
            size_t targetIdx = currentLineIdx + i;
            if (targetIdx < currentLyrics.size()) {
                if (i == 0 && isPlaying) {
                    attron(A_BOLD | COLOR_PAIR(3));
                    mvprintw(11 + i, 2, "> %.55s", currentLyrics[targetIdx].text.c_str());
                    attroff(A_BOLD | COLOR_PAIR(3));
                } else {
                    mvprintw(11 + i, 4, "%.55s", currentLyrics[targetIdx].text.c_str());
                }
            }
        }

        // 4. Estado de Temporizador e información
        mvhline(14, 0, ACS_HLINE, 60);
        std::string timerStatus = (timerMinutes > 0) ? std::to_string(timerMinutes) + "m" : "OFF";
        mvprintw(15, 1, "Estado: %s | Timer: %s | D-Pad: Navegar | A: Seleccionar", 
                 isPlaying ? "Reproduciendo" : "Detenido", timerStatus.c_str());

        refresh();

        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
                case KEY_UP:
                case 'k':
                    selected = (selected - 1 + totalItems) % totalItems;
                    break;
                case KEY_DOWN:
                case 'j':
                    selected = (selected + 1) % totalItems;
                    break;
                case 10:
                case KEY_ENTER:
                case 'a':
                case 'A': {
                    // Si seleccionó la última opción: "DETENER Y CERRAR"
                    if (selected == (int)tracks.size()) {
                        std::system("killall mpv >/dev/null 2>&1"); // Mata mpv para no consumir RAM/CPU
                        inPlayer = false;
                    } else {
                        // Seleccionó una canción
                        std::string fullPath = folderPath + "/" + tracks[selected];
                        songDuration = getAudioDuration(fullPath);
                        currentLyrics = getLyricsWithTimestamps(fullPath, songDuration, isSyncedLyrics);

                        std::string playCmd = "killall mpv >/dev/null 2>&1; mpv --no-video --no-terminal --shuffle \"" + fullPath + "\" &";
                        std::system(playCmd.c_str());

                        startTime = std::chrono::steady_clock::now();
                        isPlaying = true;
                    }
                    break;
                }
                case 27: // Tecla de emergencia para salir si estás probando en PC
                    std::system("killall mpv >/dev/null 2>&1");
                    inPlayer = false;
                    break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    nodelay(stdscr, FALSE);
}