#pragma once

#include "Module.h"
#include "ModuleWindow.h"
#include <SDL3/SDL.h>

class ModuleWindow : public Module
{
public:
    ModuleWindow();
    ~ModuleWindow();

    bool Awake();
    bool Start();
    bool PreUpdate();
    bool Update();
    bool PostUpdate();
    bool CleanUp();

    SDL_Window* GetWindow() const { return window; };
private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
};
