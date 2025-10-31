#include "DuneEngine.h"
#include "../Runetime/Window.h"
#include "../Render/Shader.h"
#include "../Render/Mesh.h"
#include "../Render/Renderer.h"

#include <SDL3/SDL.h>

static const char* kVS = R"(#version 330 core
layout(location=0) in vec3 aPos;
void main(){ gl_Position = vec4(aPos, 1.0); }
)";

static const char* kFS = R"(#version 330 core
out vec4 FragColor;
void main(){ FragColor = vec4(1.0, 0.5, 0.2, 1.0); }
)";

bool App::init() {
    m_window = std::make_unique<Window>();
    if (!m_window->create("DuneEngine", 800, 600)) return false;

    m_shader = std::make_unique<Shader>();
    if (!m_shader->compile(kVS, kFS)) return false;

    m_mesh = std::make_unique<Mesh>();
    m_mesh->createQuad();

    m_running = true;
    return true;
}

void App::run() {
    Renderer renderer;
    bool wireframe = false;

    while (m_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) m_running = false;
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_ESCAPE) m_running = false;
                else if (e.key.key == SDLK_1) wireframe = true;
                else if (e.key.key == SDLK_2) wireframe = false;
            } else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                int w, h; m_window->getDrawableSize(w, h);
                glViewport(0, 0, w, h);
            }
        }

        renderer.setWireframe(wireframe);
        renderer.clear(0.1f, 0.2f, 0.3f, 1.0f);

        m_shader->use();
        m_mesh->bind();
        m_mesh->draw();

        m_window->swap();
    }
}

void App::shutdown() {
    // Destruye en orden las RAII (unique_ptr) al salir de scope
    m_mesh.reset();
    m_shader.reset();
    m_window.reset();
    SDL_Quit();
}
