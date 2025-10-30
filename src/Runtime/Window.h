#pragma once
#include <SDL3/SDL.h>

class Window {
public:
    Window() = default;
    ~Window();

    bool create(const char* title, int width, int height);
    void swap();
    void getDrawableSize(int& w, int& h) const;
    SDL_Window* sdl() const { return m_window; }

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_gl = nullptr;
};
