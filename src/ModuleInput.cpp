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
#include "HierarchyPanel.h"
#include "Gizmo.h"
#include <SDL3/SDL_mouse.h>
#include "GameTime.h"

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
            if (event.button.button == SDL_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse && !GameTime::IsPlaying())
            {
                auto &engine = Engine::GetInstance();
                SDL_Point mp = GetMousePosition();

                ModuleScene *scene = Engine::GetInstance().scene.get();

                int width, height;
                SDL_GetWindowSizeInPixels(Engine::GetInstance().window->GetWindow(), &width, &height);

                std::shared_ptr<GameObject> selected =
                    scene->GetRaycaster()->PickObject(
                        static_cast<float>(mouseX),
                        static_cast<float>(mouseY),
                        width, height,
                        scene->GetGameObjects());

                scene->SetSelected(selected);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                auto &engine = Engine::GetInstance();
            }
            mouseButtons[event.button.button - 1] = KEY_UP;
            break;

        case SDL_EVENT_MOUSE_MOTION:
        {
            int scale = Engine::GetInstance().window.get()->GetScale();
            mouseMotionX = event.motion.xrel / static_cast<float>(scale);
            mouseMotionY = event.motion.yrel / static_cast<float>(scale);
            mouseX = event.motion.x / static_cast<float>(scale);
            mouseY = event.motion.y / static_cast<float>(scale);
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
            // Store the dropped file information for later processing by panels
            // Each panel (ProjectPanel, ScenePanel, HierarchyPanel) will check if a file was dropped
            // and handle it according to their context
            draggedFile = event.drop.data ? event.drop.data : "";
            fileDropped = true;
            dropPosition.x = static_cast<int>(mouseX);
            dropPosition.y = static_cast<int>(mouseY);
        }
        break;
        }
    }

    SDL_Window *win = Engine::GetInstance().window->GetWindow();

    bool rmb =
        mouseButtons[SDL_BUTTON_RIGHT - 1] == KEY_DOWN ||
        mouseButtons[SDL_BUTTON_RIGHT - 1] == KEY_REPEAT;

    if (rmb && !rmbRelativeMode)
    {
        SDL_SetWindowRelativeMouseMode(win, true); // oculta cursor + movimiento relativo
        SDL_SetWindowMouseGrab(win, true);
        rmbRelativeMode = true;
    }
    else if (!rmb && rmbRelativeMode)
    {
        SDL_SetWindowRelativeMouseMode(win, false); // muestra cursor
        SDL_SetWindowMouseGrab(win, false);
        rmbRelativeMode = false;
    }

    // Handle keyboard shortcuts
    HandleKeyboardShortcuts();

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

void ModuleInput::HandleKeyboardShortcuts()
{
    // Ctrl+D to Duplicate
    if (GetKey(SDL_SCANCODE_D) == KEY_DOWN && GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
    {
        ModuleScene *scene = Engine::GetInstance().scene.get();
        if (scene)
        {
            std::shared_ptr<GameObject> selected = scene->GetSelected();
            if (selected)
            {
                // Duplicate the GameObject
                auto duplicated = scene->CreateGameObject();
                duplicated->SetName(scene->GenerateUniqueName(selected->GetName()));

                for (auto &comp : selected->GetComponents())
                {
                    duplicated->CreateComponent(comp->GetType());
                }
                for (auto &child : selected->GetChildren())
                {
                    scene->DuplicateGameObject(child, duplicated);
                }
                // Set parent if selected has one
                if (auto parent = selected->GetParent())
                {
                    duplicated->SetParent(parent);
                }
            }
        }
    }
    if (GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
    {
        ModuleScene *scene = Engine::GetInstance().scene.get();
        if (scene)
        {
            scene->SaveScene("Assets/Scenes/TestScene.scene");
            LOG_INFO("Scene saved");
        }
    }

    // F9 -> Load Scene
    if (GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
    {
        ModuleScene *scene = Engine::GetInstance().scene.get();
        if (scene)
        {
            scene->LoadScene("Assets/Scenes/TestScene.scene");
            LOG_INFO("Scene loaded");
        }
    }

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        // Bloquear mientras RMB está siendo usado para mover cámara
        bool rmbDown =
            GetMouseButtonDown(3) == KEY_DOWN ||
            GetMouseButtonDown(3) == KEY_REPEAT;

        if (!rmbDown)
        {
            auto &engine = Engine::GetInstance();
            ModuleScene *scene = engine.scene.get();
            bool hasSelection = (scene && scene->GetSelected() != nullptr);
        }
    }
    // F11 to Toggle Fullscreen
    if (GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
    {
        Engine::GetInstance().window.get()->ToggleFullscreen();
    }
}

void ModuleInput::HandleFileDrop(const SDL_Event &event)
{
    draggedFile = event.drop.data ? event.drop.data : "";
    fileDropped = true;
    dropPosition.x = static_cast<int>(mouseX);
    dropPosition.y = static_cast<int>(mouseY);

    if (ResourceUtils::GetTypeFromExtension(draggedFile) == ResourceType::Model)
    {
        std::shared_ptr<GameObject> gameObject = Engine::GetInstance().scene.get()->CreateGameObject();
        gameObject->SetName(std::filesystem::path(draggedFile).filename().stem().string());

        std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;
        std::shared_ptr<Resource> resource = resourceManager->RequestResource(draggedFile);
        std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(resource);

        for (size_t i = 0; i < model->GetMeshes().size(); ++i)
        {
            auto &mesh = model->GetMeshes()[i];

            gameObject->CreateComponent(ComponentType::Mesh);
            MeshComponent *meshComponent = gameObject->GetComponent<MeshComponent>();
            meshComponent->SetMesh(mesh);

            gameObject->CreateComponent(ComponentType::Material);
            MaterialComponent *materialComponent = gameObject->GetComponent<MaterialComponent>();
            materialComponent->SetMaterial(std::make_shared<Material>(ResourceType::Material));
        }
    }

    Engine::GetInstance().scene->RebuildQuadtree();
}
