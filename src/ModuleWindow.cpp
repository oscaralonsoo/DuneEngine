#include "ModuleWindow.h"
#include "Engine.h"
#include "Globals.h"
#include <glad/glad.h>
#include "Shader.h"
#include "RendererAPI.h"
#include <fstream>
#include <vector>
#include <string>

ModuleWindow::ModuleWindow() : Module()
{
    name = "window";
}

bool ModuleWindow::Awake()
{
    LOG_INFO("Init SDL window & surface");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_ERROR("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // --- Create Window ---
    // Create SDL window with resizing
    window = SDL_CreateWindow("DuneEngine", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        LOG_ERROR("Window could not be created! SDL_Error: %s\n", SDL_GetError());
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

    SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_BEGIN, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_COMPLETE, true);

    SDL_SetWindowRelativeMouseMode(window, false);

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

    SDL_GetWindowSizeInPixels(window, &width, &height);
    RendererAPI::SetViewport(0, 0, width, height);

    return true;
}

bool ModuleWindow::Start()
{
    return true;
}

bool ModuleWindow::PreUpdate()
{
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

void ModuleWindow::SetTitle(const char* new_title)
{
	SDL_SetWindowTitle(window, new_title);
}

std::pair<int,int> ModuleWindow::GetWindowSize() const
{
    return { width, height };
}

void ModuleWindow::SetSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

int ModuleWindow::GetScale() const
{
	return scale;
}