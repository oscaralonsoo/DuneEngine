#pragma once
#include "Module.h"
#include "Globals.h"
#include "shader_s.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Application;

class ModuleWindow : public Module
{
public:
    ModuleWindow(Application* app);
    ~ModuleWindow() override;

    bool Awake() override;
    bool Start() override;
    bool PreUpdate() override;
    bool Update() override;
    bool PostUpdate() override;
    bool CleanUp() override;

private:
    Application* app = nullptr;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    Shader* shader = nullptr;
    GLuint VAO = 0, VBO = 0;
    GLuint texture1 = 0, texture2 = 0;
    glm::mat4 model, view, projection;
};
