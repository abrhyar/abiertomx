#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> // Para system()

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

struct MenuItem {
    std::string title;
    std::string action; // Comando a ejecutar
};

// Función para ejecutar comandos pausando temporalmente la ventana SDL
void executeCommand(SDL_Window* window, const std::string& command) {
    if (command.empty()) return;

    // Minimizar/Ocultar la ventana de ALk para dar paso a la app externa
    SDL_HideWindow(window);

    // Ejecutar el comando en el sistema
    std::system(command.c_str());

    // Al cerrar la app externa, volvemos a mostrar la ventana de ALk
    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return 1;
    }

    if (TTF_Init() < 0) {
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "ALk Launcher",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_ShowCursor(SDL_DISABLE);

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);
    }

    std::vector<MenuItem> items = {
        {"Juegos (GBA, SNES)", ""},
        {"Musica", ""},
        {"Videos", ""},
        {"Fotos", ""},
        // Inicia xterm a pantalla completa ejecutando armbian-config
        {"Configuracion", "sudo xterm -fullscreen -e armbian-config"}, 
        {"Salir", "EXIT"}
    };

    int selected = 0;
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        selected = (selected - 1 + (int)items.size()) % (int)items.size();
                        break;
                    case SDLK_DOWN:
                        selected = (selected + 1) % (int)items.size();
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if (items[selected].action == "EXIT") {
                            running = false;
                        } else if (!items[selected].action.empty()) {
                            executeCommand(window, items[selected].action);
                        }
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }

        // Renderizado de UI
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Header
        if (font) {
            SDL_Color headerColor = {0, 255, 200, 255};
            SDL_Surface* headSurf = TTF_RenderUTF8_Blended(font, "ABIERTO v0.1 | ALk", headerColor);
            if (headSurf) {
                SDL_Texture* headTex = SDL_CreateTextureFromSurface(renderer, headSurf);
                SDL_Rect headRect = {10, 10, headSurf->w, headSurf->h};
                SDL_RenderCopy(renderer, headTex, NULL, &headRect);
                SDL_FreeSurface(headSurf);
                SDL_DestroyTexture(headTex);
            }
        }

        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawLine(renderer, 10, 32, 310, 32);

        // Opciones del Menú
        for (size_t i = 0; i < items.size(); ++i) {
            bool isSelected = ((int)i == selected);

            if (isSelected) {
                SDL_Rect selectBox = {10, (int)(42 + i * 26), 300, 22};
                SDL_SetRenderDrawColor(renderer, 0, 120, 215, 255);
                SDL_RenderFillRect(renderer, &selectBox);
            }

            if (font) {
                SDL_Color textColor = isSelected ? SDL_Color{255, 255, 255, 255} : SDL_Color{180, 180, 180, 255};
                SDL_Surface* surface = TTF_RenderUTF8_Blended(font, items[i].title.c_str(), textColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_Rect dstRect = {20, (int)(44 + i * 26), surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
                    SDL_FreeSurface(surface);
                    SDL_DestroyTexture(texture);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawLine(renderer, 10, 210, 310, 210);

        // Footer
        if (font) {
            SDL_Color footerColor = {120, 120, 120, 255};
            SDL_Surface* footSurf = TTF_RenderUTF8_Blended(font, "[^v] Navegar   [ENTER] Entrar", footerColor);
            if (footSurf) {
                SDL_Texture* footTex = SDL_CreateTextureFromSurface(renderer, footSurf);
                SDL_Rect footRect = {10, 218, footSurf->w, footSurf->h};
                SDL_RenderCopy(renderer, footTex, NULL, &footRect);
                SDL_FreeSurface(footSurf);
                SDL_DestroyTexture(footTex);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}