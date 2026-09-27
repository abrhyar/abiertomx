#include "camera.hpp"
#include "utils.hpp"
#include <ncurses.h>
#include <vector>
#include <string>
#include <cstdlib>

void recordVideoInteractive() {
    clear();
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(0, 1, "ABIERTO | Grabando Video");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvhline(1, 0, ACS_HLINE, 38);
    
    attron(A_BOLD);
    mvprintw(5, 4, "[ REC ] GRABANDO VIDEO...");
    attroff(A_BOLD);
    
    mvprintw(7, 2, "Presiona [ENTER] o [ESC]");
    mvprintw(8, 2, "para detener...");

    mvhline(12, 0, ACS_HLINE, 38);
    mvprintw(13, 1, "[ESC/ENTER] Detener y Guardar");
    refresh();

    // Iniciar ffmpeg en segundo plano guardando el PID
    std::string startCmd = "ffmpeg -y -f v4l2 -i /dev/video1 ~/Videos/vid-$(date +%H%M%S-%d-%m-%Y).mp4 > /dev/null 2>&1 & echo $! > /tmp/ffmpeg_rec.pid";
    std::system(startCmd.c_str());

    // Esperar a que el usuario presione una tecla para detener
    getch();

    // Enviar señal SIGINT (equivalente a Ctrl+C) a ffmpeg para cerrar el MP4 adecuadamente
    std::system("kill -SIGINT $(cat /tmp/ffmpeg_rec.pid) 2>/dev/null");
    std::system("rm -f /tmp/ffmpeg_rec.pid");

    clear();
    mvprintw(6, 4, "Video guardado en ~/Videos");
    refresh();
    napms(1200);
}

void showCameraView() {
    std::vector<MenuItem> camOptions = {
        {"[PRE]", "Vista Previa (ffplay)", "VIEW"},
        {"[REC]", "Grabar Video", "REC"},
        {"[PIC]", "Tomar Foto", "PIC"}
    };

    int selected = 0;
    bool inCamMenu = true;

    while (inCamMenu) {
        clear();

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(0, 1, "ABIERTO | Camara USB");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvhline(1, 0, ACS_HLINE, 38);

        int startY = 3;
        for (size_t i = 0; i < camOptions.size(); ++i) {
            if ((int)i == selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(startY + i, 1, "> %-5s %-26s", camOptions[i].tag.c_str(), camOptions[i].title.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(startY + i, 3, "%-5s %s", camOptions[i].tag.c_str(), camOptions[i].title.c_str());
            }
        }

        mvhline(12, 0, ACS_HLINE, 38);
        mvprintw(13, 1, "[^v] Navegar  [ENT] Ejecutar  [ESC] Atras");

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                selected = (selected - 1 + (int)camOptions.size()) % (int)camOptions.size();
                break;
            case KEY_DOWN:
            case 'j':
                selected = (selected + 1) % (int)camOptions.size();
                break;
            case 10:
            case KEY_ENTER:
                if (camOptions[selected].command == "VIEW") {
                    std::string cmd = "ffplay -f v4l2 -input_format mjpeg -video_size 640x480 /dev/video1";
                    runCommand(cmd);
                } else if (camOptions[selected].command == "REC") {
                    recordVideoInteractive();
                } else if (camOptions[selected].command == "PIC") {
                    std::string cmd = "ffmpeg -y -f v4l2 -i /dev/video1 -vframes 1 ~/Pictures/img-$(date +%H%M%S-%d-%m-%Y).jpg";
                    runCommand(cmd);
                }
                break;
            case 27:
            case 'q':
                inCamMenu = false;
                break;
        }
    }
}