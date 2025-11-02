#include "ModuleWindow.h"
#include "Engine.h"

#include <glad/glad.h>
#include "shader_s.h"
#include <fstream>
#include <vector>
#include <string>

ModuleWindow::ModuleWindow() : Module()
{
    name = "window";
}

ModuleWindow::~ModuleWindow()
{
}

bool ModuleWindow::Awake()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // --- Create Window ---
    // Create SDL window with OpenGL support
    window = SDL_CreateWindow("DuneEngine", 800, 600, SDL_WINDOW_OPENGL);
    if (!window)
    {
        SDL_Quit();
        return false;
    }

    // --- Create OpenGL Context ---
    // Create a rendering context for the window
    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // --- Load OpenGL Functions ---
    // Initialize GLAD to load OpenGL function pointers
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
    {
        SDL_Log("Failed to initialize GLAD\n");
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    return true;
}

bool ModuleWindow::Start()
{
    return true;
}

bool ModuleWindow::PreUpdate()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            //app->running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE)
            {
                //app->running = false;
            }
            else if (event.key.key == SDLK_1)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
            }
            else if (event.key.key == SDLK_2)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Fill mode
            }
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            int w, h;
            SDL_GetWindowSizeInPixels(window, &w, &h);
            glViewport(0, 0, w, h);
            break;
        }
    }
    return true;
}

bool ModuleWindow::Update()
{
    return true;
}

bool ModuleWindow::PostUpdate()
{
    return true;
}

bool ModuleWindow::CleanUp()
{
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return true;
}