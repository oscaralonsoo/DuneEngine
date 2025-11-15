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

    void SetTitle(const char* title);
	void SetWindowSize(int& width, int& height) const;

	int GetScale() const;
    SDL_Window* GetWindow() const { return window; };
    SDL_GLContext GetGLContext() const { return glContext; }
    
private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

	std::string title;
	int width = 1280;
	int height = 720;
	int scale = 1;
};
