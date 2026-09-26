#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    // 1. Inicializar SDL2, Image y TTF
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Error iniciando SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "Error iniciando SDL_ttf: " << TTF_GetError() << std::endl;
        return 1;
    }

    // 2. Crear ventana e interfaz X11
    SDL_Window* window = SDL_CreateWindow(
        "Abierto UI",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    // 3. Cargar Fuente TTF (Buscamos la fuente de sistema de Linux)
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    SDL_Texture* textTexture = nullptr;
    SDL_Rect textRect;

    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, "PROYECTO ABIERTO - X11 KIOSK", white);
        if (textSurface) {
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            textRect = { (SCREEN_WIDTH - textSurface->w) / 2, 100, textSurface->w, textSurface->h };
            SDL_FreeSurface(textSurface);
        }
    } else {
        std::cout << "Aviso: No se encontro la fuente de sistema, se mostrara pantalla sin texto." << std::endl;
    }

    bool running = true;
    SDL_Event event;

    // 4. Bucle principal de la interfaz
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

        // Fondo oscuro tipo UI de consola
        SDL_SetRenderDrawColor(renderer, 18, 20, 26, 255);
        SDL_RenderClear(renderer);

        // Renderizar Texto
        if (textTexture) {
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        }

        SDL_RenderPresent(renderer);
    }

    // Limpieza de memoria
    if (textTexture) SDL_DestroyTexture(textTexture);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}