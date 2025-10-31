#include "Window.h"
#include <glad/glad.h>

Window::~Window() {
    if (m_gl) { SDL_GL_DestroyContext(m_gl); m_gl = nullptr; }
    if (m_window) { SDL_DestroyWindow(m_window); m_window = nullptr; }
}

bool Window::create(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    // OpenGL 3.3 Core (tus shaders usan #version 330)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    m_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    m_gl = SDL_GL_CreateContext(m_window);
    if (!m_gl) {
        SDL_Log("SDL_GL_CreateContext failed: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        SDL_Log("Failed to init GLAD");
        return false;
    }

    int w, h; getDrawableSize(w, h);
    glViewport(0, 0, w, h);

    SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("OpenGL:   %s", glGetString(GL_VERSION));
    SDL_Log("GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}

void Window::swap() {
    SDL_GL_SwapWindow(m_window);
}

void Window::getDrawableSize(int& w, int& h) const {
    SDL_GetWindowSizeInPixels(m_window, &w, &h);
}
