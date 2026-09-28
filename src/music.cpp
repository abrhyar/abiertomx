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
return 180.0; // 3 minutos por defecto si no detecta
}

// Extraer letras y analizar si están sincronizadas con formato [mm:ss.xx]
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

// Si no es sincronizada, distribuimos las líneas proporcionalmente al tiempo total
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

auto startTime = std::chrono::steady_clock::now();
double songDuration = 0.0;

nodelay(stdscr, TRUE);

while (inPlayer) {
// Usamos erase() en lugar de clear() para evitar el parpadeo
erase();

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

// 1. Lista de Pistas (Ocupa la parte superior)
mvprintw(2, 1, "+--- PISTAS EN ~/Music ---+");
int startY = 3;
for (size_t i = 0; i < tracks.size() && i < 5; ++i) {
if ((int)i == selected) {
attron(COLOR_PAIR(1) | A_BOLD);
mvprintw(startY + i, 1, "> %-58s", tracks[i].substr(0, 58).c_str());
attroff(COLOR_PAIR(1) | A_BOLD);
} else {
mvprintw(startY + i, 3, "%-58s", tracks[i].substr(0, 58).c_str());
}
}

// 2. Cuadro de Letras (Ocupa la parte central/inferior)
mvprintw(9, 1, "+--- LETRAS (AUTOSCROLL) ---+");

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

// Dibujar 3 líneas de letras centradas en la pantalla
for (size_t i = 0; i < 3; ++i) {
size_t targetIdx = currentLineIdx + i;
if (targetIdx < currentLyrics.size()) {
if (i == 0 && isPlaying) {
attron(A_BOLD | COLOR_PAIR(3));
mvprintw(10 + i, 2, "> %.60s", currentLyrics[targetIdx].text.c_str());
attroff(A_BOLD | COLOR_PAIR(3));
} else {
mvprintw(10 + i, 4, "%.60s", currentLyrics[targetIdx].text.c_str());
}
}
}

// 3. Barra de Estado
mvhline(14, 0, ACS_HLINE, 65);
std::string statusStr = isPlaying ? "[PLAYING]" : "[PAUSED]";
std::string modeStr = isSyncedLyrics ? "[LRC:SYNC]" : "[LRC:AUTO]";
mvprintw(15, 1, "%s %s [ENT] Play  [S] Random  [ESC] Atras", statusStr.c_str(), modeStr.c_str());

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

std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

nodelay(stdscr, FALSE);
}