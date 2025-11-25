#include "ModuleInput.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Model.h"
#include "Globals.h"
#include "RendererAPI.h"
#include "MaterialComponent.h"
#include "ResourceUtils.h"
#include <string.h>
#include <filesystem>

#define MAX_KEYS 300

ModuleInput::ModuleInput() : Module()
{
    name = "input";

    keyboard = new KeyState[MAX_KEYS];
    memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
    memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
}

ModuleInput::~ModuleInput()
{
    delete[] keyboard;
}

bool ModuleInput::Awake()
{
    LOG_INFO("Init SDL input event system");
    bool ret = true;
    SDL_Init(0);

    if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
    {
        LOG_ERROR("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
        ret = false;
    }
    return ret;
}

bool ModuleInput::Start()
{
    SDL_StopTextInput(Engine::GetInstance().window.get()->GetWindow());
    return true;
}

bool ModuleInput::PreUpdate()
{
    const bool *keys = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < MAX_KEYS; ++i)
    {
        if (keys[i])
            keyboard[i] = (keyboard[i] == KEY_IDLE) ? KEY_DOWN : KEY_REPEAT;
        else
            keyboard[i] = (keyboard[i] == KEY_DOWN || keyboard[i] == KEY_REPEAT) ? KEY_UP : KEY_IDLE;
    }

    for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
    {
        if (mouseButtons[i] == KEY_DOWN)
            mouseButtons[i] = KEY_REPEAT;
        if (mouseButtons[i] == KEY_UP)
            mouseButtons[i] = KEY_IDLE;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            windowEvents[WE_QUIT] = true;
            break;

        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            windowEvents[WE_HIDE] = true;
            break;

        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
            windowEvents[WE_SHOW] = true;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            mouseButtons[event.button.button - 1] = KEY_DOWN;

            if (event.button.button == SDL_BUTTON_LEFT)
            {
                ModuleScene *scene = Engine::GetInstance().scene.get();

                std::shared_ptr<GameObject> picked = scene->GetRaycaster()->PickObject(mouseX, mouseY, scene->GetGameObjects());

                if (picked)
                {
                    scene->SetSelected(picked);
                }
                else
                {
                    scene->SetSelected(nullptr);
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            mouseButtons[event.button.button - 1] = KEY_UP;
            break;

        case SDL_EVENT_MOUSE_MOTION:
        {
            int scale = Engine::GetInstance().window.get()->GetScale();
            mouseMotionX = event.motion.xrel / scale;
            mouseMotionY = event.motion.yrel / scale;
            mouseX = event.motion.x / scale;
            mouseY = event.motion.y / scale;
        }
        break;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            int w, h;
            SDL_GetWindowSizeInPixels(Engine::GetInstance().window.get()->GetWindow(), &w, &h);

            Engine::GetInstance().window.get()->SetSize(w, h); // Update size in ModuleWindow

            RendererAPI::SetViewport(0, 0, w, h); // Update del viewport
        }
        break;

        case SDL_EVENT_DROP_FILE:
        {
            // ─────────── FIXME ───────────
            // TODO: Mover a EventSystem
            // ────────────────────────────
            const std::string &file = event.drop.data ? event.drop.data : "";

            if (ResourceUtils::GetTypeFromExtension(file) == ResourceType::Model)
            {
                std::shared_ptr<GameObject> go = Engine::GetInstance().scene.get()->CreateGameObject();
                go->SetName(std::filesystem::path(file).filename().stem().string());

                std::shared_ptr<Model> model = std::make_shared<Model>(file);

                for (size_t i = 0; i < model->GetMeshes().size(); ++i)
                {
                    auto &mesh = model->GetMeshes()[i];

                    go->CreateComponent(ComponentType::Mesh);
                    MeshComponent *meshComp = go->GetComponent<MeshComponent>();
                    meshComp->SetMesh(mesh);

                    go->CreateComponent(ComponentType::Material);
                    MaterialComponent *materialComp = go->GetComponent<MaterialComponent>();
                    materialComp->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                }
            }
        }
        break;
        }
    }

    return true;
}

bool ModuleInput::CleanUp()
{
    LOG_INFO("Quitting SDL event subsystem");
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    return true;
}

bool ModuleInput::GetWindowEvent(EventWindow ev) const
{
    return windowEvents[ev];
}

SDL_Point ModuleInput::GetMousePosition() const
{
    return {mouseX, mouseY};
}

SDL_Point ModuleInput::GetMouseMotion() const
{
    return {mouseMotionX, mouseMotionY};
}
