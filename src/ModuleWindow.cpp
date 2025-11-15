#include "ModuleWindow.h"
#include "Engine.h"
#include "Globals.h"
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
    // Create SDL window with OpenGL support
    window = SDL_CreateWindow("DuneEngine", 800, 600, SDL_WINDOW_OPENGL);
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

void ModuleWindow::SetWindowSize(int& width, int& height) const
{
	width = this->width;
	height = this->height;
}

int ModuleWindow::GetScale() const
{
	return scale;
}