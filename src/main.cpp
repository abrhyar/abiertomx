#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

struct MenuItem {
    std::string title;
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return 1;
    }
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("ALk Launcher", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_ShowCursor(SDL_DISABLE); // Ocultar el cursor del mouse

    // Cargar fuente TTF del sistema
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);

    std::vector<MenuItem> items = {
        {"Juegos (GBA, SNES)"},
        {"Musica"},
        {"Videos"},
        {"Fotos"},
        {"Configuracion"},
        {"Salir"}
    };

    int selected = 0;
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        selected = (selected - 1 + items.size()) % items.size();
                        break;
                    case SDLK_DOWN:
                        selected = (selected + 1) % items.size();
                        break;
                    case SDLK_RETURN:
                        if (selected == (int)items.size() - 1) running = false;
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }

        // Fondo Negro
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dibujar Menú
        for (size_t i = 0; i < items.size(); ++i) {
            SDL_Color color = (i == (size_t)selected) ? SDL_Color{0, 162, 232, 255} : SDL_Color{255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, items[i].title.c_str(), color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_Rect dstRect = {20, (int)(40 + i * 28), surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &dstRect);

            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}