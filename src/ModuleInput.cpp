#include "ModuleInput.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Model.h"
#include "Globals.h"
#include "RendererAPI.h"
#include "MaterialComponent.h"
#include "ResourceUtils.h"
#include "ResourceImporter.h"
#include "ModuleResource.h"
#include "TextureImportData.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <string.h>
#include <filesystem>

#define MAX_KEYS 300

ModuleInput::ModuleInput() : Module()
{
    name = "input";

    keyboard = new KeyState[MAX_KEYS];
    memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
    memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
    draggedFile = "";
    fileDropped = false;
    dropPosition = {0, 0};
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
    // Update keyboard states
    const bool *keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        if (keys[i])
            keyboard[i] = (keyboard[i] == KEY_IDLE) ? KEY_DOWN : KEY_REPEAT;
        else
            keyboard[i] = (keyboard[i] == KEY_DOWN || keyboard[i] == KEY_REPEAT) ? KEY_UP : KEY_IDLE;
    }

    // Update mouse button states
    for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
    {
        if (mouseButtons[i] == KEY_DOWN)
            mouseButtons[i] = KEY_REPEAT;
        if (mouseButtons[i] == KEY_UP)
            mouseButtons[i] = KEY_IDLE;
    }

    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);

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

            // Handle object picking on left mouse button click
            if (event.button.button == SDL_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
            {
                ModuleScene *scene = Engine::GetInstance().scene.get();

                std::shared_ptr<GameObject> selected = scene->GetRaycaster()->PickObject(mouseX, mouseY, scene->GetGameObjects());
                if (selected)
                {
                    scene->SetSelected(selected);
                }
                else
                {
                    scene->ResetSelecteds();
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
            RendererAPI::SetViewport(0, 0, w, h);              // Update viewport
        }
        break;

        case SDL_EVENT_DROP_FILE:
        {
            const std::string &file = event.drop.data ? event.drop.data : "";

            ResourceType type = ResourceUtils::GetTypeFromExtension(file);

            if (type == ResourceType::Model)
            {
                std::shared_ptr<Model> model = std::make_shared<Model>(file);

                for (size_t i = 0; i < model->GetMeshes().size(); ++i)
                {
                    std::shared_ptr<GameObject> go = Engine::GetInstance().scene.get()->CreateGameObject();
                    go->SetName(ResourceUtils::ToString(ResourceType::Model));

                    auto &mesh = model->GetMeshes()[i];

                    go->CreateComponent(ComponentType::Mesh);
                    MeshComponent *meshComp = go->GetComponent<MeshComponent>();
                    meshComp->SetMesh(mesh);

                    go->CreateComponent(ComponentType::Material);
                    MaterialComponent *materialComp = go->GetComponent<MaterialComponent>();
                    materialComp->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                }
            }
            else if (type == ResourceType::Texture)
            {
                ModuleScene *scene = Engine::GetInstance().scene.get();
                std::shared_ptr<GameObject> selected = scene->GetRaycaster()->PickObject(event.drop.x, event.drop.y, scene->GetGameObjects());
                scene->SetSelected(selected);

                if (!selected)
                    break;

                MaterialComponent *materialComp = selected->GetComponent<MaterialComponent>();
                if (!materialComp)
                    break;

                std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;
                std::shared_ptr<Resource> resource = resourceManager->RequestResource(file);
                std::shared_ptr<Texture> texture = std::dynamic_pointer_cast<Texture>(resource);
                materialComp->GetMaterial()->SetTexture(TextureType::Albedo, texture);

                ;
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

// Handles file drop events
void ModuleInput::HandleFileDrop(const SDL_Event &event)
{
    draggedFile = event.drop.data ? event.drop.data : "";
    fileDropped = true;
    dropPosition = {mouseX, mouseY};

    if (ResourceUtils::GetTypeFromExtension(draggedFile) == ResourceType::Model)
    {
        std::shared_ptr<GameObject> go = Engine::GetInstance().scene.get()->CreateGameObject();
        go->SetName(std::filesystem::path(draggedFile).filename().stem().string());

        std::shared_ptr<Model> model = std::make_shared<Model>(draggedFile);

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
