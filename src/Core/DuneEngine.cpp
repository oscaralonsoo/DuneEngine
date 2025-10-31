#include "DuneEngine.h"
#include "../Runtime/Window.h"
#include "../Render/Shader.h"
#include "../Assets/Mesh.h"
#include "../Render/Renderer.h"

#include <SDL3/SDL.h>
#include <iostream>


static const char* kVS = R"(#version 140
in vec3 aPos;
in vec3 aColor;
out vec3 vColor;

// MVP si ya lo usas; si no, deja identidad y listo
uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

static const char* kFS = R"(#version 140
in vec3 vColor;
out vec4 FragColor;

// multiplicador de color (para líneas traseras)
uniform vec3 uTint; // por defecto (1,1,1)

void main() {
    FragColor = vec4(vColor * uTint, 1.0);
}
)";

bool App::init() {
    m_window = std::make_unique<Window>();
    if (!m_window->create("DuneEngine", 800, 600)) return false;

    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_unique<Shader>();
    if (!m_shader->compile(kVS, kFS)) return false;

    m_mesh = std::make_unique<Mesh>();
    m_mesh->CreateCube();

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
                switch (e.key.key) {
                    case SDLK_ESCAPE: m_running = false; break;
                    case SDLK_1: m_renderMode = RenderMode::WireHidden; break;
                    case SDLK_2: m_renderMode = RenderMode::Solid; break;
                }
            } else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                int w, h; m_window->getDrawableSize(w, h);
                glViewport(0, 0, w, h);
            }
        }

        renderer.setWireframe(wireframe);
        renderer.clear(0.1f, 0.2f, 0.3f, 1.0f);

        m_shader->use();

         m_renderer.renderMesh(*m_mesh, *m_shader, m_renderMode);

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
