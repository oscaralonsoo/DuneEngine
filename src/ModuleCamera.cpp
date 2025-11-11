#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "Globals.h"
#include "Engine.h"
#include "sdl3/SDL.h"

ModuleCamera::ModuleCamera() : Module()
{
    name = "camera";
}

ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::Awake()
{
    camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    return true;
}

bool ModuleCamera::Start()
{
    return true;
}

bool ModuleCamera::PreUpdate()
{
    return true;
}

bool ModuleCamera::Update()
{
    auto input = Engine::GetInstance().input.get();

    if (input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN ||
        input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
    {
        float speedMultiplier = (input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || 
                                 input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT) ? 2.0f : 1.0f;

        if (input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
            camera.ProcessKeyboard(FORWARD, /*deltaTime **/0.01* speedMultiplier);
        if (input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
            camera.ProcessKeyboard(BACKWARD, /*deltaTime **/0.01* speedMultiplier);
        if (input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
            camera.ProcessKeyboard(LEFT, /*deltaTime **/0.01*speedMultiplier);
        if (input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
            camera.ProcessKeyboard(RIGHT, /*deltaTime **/0.01*speedMultiplier);
    }

    return true;
}




bool ModuleCamera::PostUpdate()
{
    return true;
}

bool ModuleCamera::CleanUp()
{
    return true;
}
