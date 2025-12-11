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

                std::shared_ptr<GameObject> selected =
                scene->GetRaycaster()->PickObject(
                    static_cast<float>(mouseX),
                    static_cast<float>(mouseY),
                    scene->GetGameObjects());

                if (selected)
                {
                    scene->SetSelected(selected);
                } else {
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
            mouseMotionX = event.motion.xrel / static_cast<float>(scale);
            mouseMotionY = event.motion.yrel / static_cast<float>(scale);
            mouseX       = event.motion.x    / static_cast<float>(scale);
            mouseY       = event.motion.y    / static_cast<float>(scale);
        }
        break;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            int w, h;
            SDL_GetWindowSizeInPixels(Engine::GetInstance().window.get()->GetWindow(), &w, &h);

            Engine::GetInstance().window.get()->SetSize(w, h); // Update size in ModuleWindow
            RendererAPI::SetViewport(0, 0, w, h); // Update viewport
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

                Engine::GetInstance().scene->RebuildQuadtree();
            }
            else if (type == ResourceType::Texture)
            {
                ModuleScene *scene = Engine::GetInstance().scene.get();
                std::shared_ptr<GameObject> selected =
                scene->GetRaycaster()->PickObject(
                    static_cast<float>(event.drop.x),
                    static_cast<float>(event.drop.y),
                    scene->GetGameObjects());

                scene->SetSelected(selected);

                if (!selected)
                    break;

                MaterialComponent *materialComp = selected->GetComponent<MaterialComponent>();
                if (!materialComp)
                    break;

                std::shared_ptr<Texture> texture = std::make_shared<Texture>(file);
                materialComp->GetMaterial()->SetTexture(TextureType::Albedo, texture);
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
    return true;
}

bool ModuleInput::GetWindowEvent(EventWindow ev) const
{
    return windowEvents[ev];
}

SDL_Point ModuleInput::GetMousePosition() const
{
    SDL_Point p;
    p.x = static_cast<int>(mouseX);
    p.y = static_cast<int>(mouseY);
    return p;
}

SDL_Point ModuleInput::GetMouseMotion() const
{
    SDL_Point p;
    p.x = static_cast<int>(mouseMotionX);
    p.y = static_cast<int>(mouseMotionY);
    return p;
}

// Handles file drop events
void ModuleInput::HandleFileDrop(const SDL_Event& event)
{
    draggedFile = event.drop.data ? event.drop.data : "";
    fileDropped = true;
    dropPosition.x = static_cast<int>(mouseX);
    dropPosition.y = static_cast<int>(mouseY);

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

    Engine::GetInstance().scene->RebuildQuadtree();
}
