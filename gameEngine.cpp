#include <fmt/core.h>
#include "SDL3/SDL.h"

int main()
{
    fmt::print("Iniciando SDL3...\n");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fmt::print("Error al inicializar SDL: {}\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Ventana SDL3 - Cierra cuando quieras", 800, 600, 0);
    if (!window) {
        fmt::print("Error al crear la ventana: {}\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    fmt::print("Ventana creada. Cierra la ventana para salir.\n");

    bool running = true;
    SDL_Event event;

    while (running) {
        // Espera hasta que ocurra un evento (bloqueante, eficiente)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Aquí podrías agregar renderizado o lógica si quisieras
        SDL_Delay(16); // ~60 FPS (opcional)
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    fmt::print("Ventana cerrada. Programa terminado.\n");
    return 0;
}
