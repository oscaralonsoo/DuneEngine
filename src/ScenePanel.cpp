#include "ScenePanel.h"
#include "SceneViewController.h"
#include "SceneInteractionHandler.h"
#include "SceneAssetHandler.h"
#include "ScenePanelUI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleRenderer.h"
#include "Renderer.h"
#include "EditorCamera.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "GameTime.h"
#include "Gizmo.h"
#include <imgui.h>

ScenePanel::ScenePanel()
    : m_ViewController(std::make_unique<SceneViewController>())
    , m_InteractionHandler(std::make_unique<SceneInteractionHandler>())
    , m_AssetHandler(std::make_unique<SceneAssetHandler>())
    , m_UI(std::make_unique<ScenePanelUI>())
{
}

ScenePanel::~ScenePanel() = default;

bool ScenePanel::Start()
{
    return m_ViewController->Initialize(800, 600);
}

void ScenePanel::OnImGuiRender()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Scene", nullptr, flags);

    RenderToolbarControls();
    ImGui::Separator();

    bool isPlaying = GameTime::IsPlaying();
    ViewType currentView = m_ViewController->GetCurrentView();
    bool forceRestore = m_ViewController->ShouldForceRestoreView();

    // Render tab bar with view switching
    m_UI->RenderTabBar(
        currentView,
        isPlaying,
        forceRestore,
        [this](ViewType newView) {
            m_ViewController->SetCurrentView(newView);
        }
    );

    // Render appropriate view content
    if (currentView == ViewType::Scene && !isPlaying)
    {
        RenderSceneView();
    }
    else if (currentView == ViewType::Game)
    {
        RenderGameView();
    }

    m_ViewController->ClearForceRestoreView();

    ImGui::End();
}

void ScenePanel::RenderSceneView()
{
    auto& engine = Engine::GetInstance();
    auto* framebuffer = m_ViewController->GetSceneFramebuffer();
    
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    uint32_t width = (uint32_t)contentRegion.x;
    uint32_t height = (uint32_t)contentRegion.y;

    // Setup gizmo viewport
    if (engine.gizmo)
        engine.gizmo->SetViewportSize((int)width, (int)height);

    // Resize framebuffer if needed
    m_ViewController->ResizeFramebufferIfNeeded(framebuffer, width, height);

    // Render scene to framebuffer
    auto* renderer = engine.renderer.get();
    if (renderer && renderer->editorCamera)
    {
        bool sceneEditable = m_InteractionHandler->IsSceneEditable();
        renderer->editorCamera->SetInputEnabled(sceneEditable);
        renderer->RenderToFramebuffer(framebuffer, renderer->editorCamera);
    }

    // Render UI with callbacks for interactions
    m_UI->RenderSceneView(
        framebuffer,
        [this, width, height](float mouseX, float mouseY, bool isClick) {
            if (isClick)
            {
                m_InteractionHandler->HandleMouseClick(mouseX, mouseY, width, height);
            }
            else if (mouseX < 0.0f || mouseY < 0.0f)
            {

                m_InteractionHandler->HandleMouseRelease();
            }
            else
            {

                m_InteractionHandler->UpdateGizmo(mouseX, mouseY);
            }
        },
        [this]() {
            m_InteractionHandler->HandleKeyboardShortcuts();
        },
        [this, framebuffer](ImVec2 viewSize, float mouseX, float mouseY) {
            m_AssetHandler->HandleDragDrop(viewSize, mouseX, mouseY, framebuffer);
        }
    );
}

void ScenePanel::RenderGameView()
{
    auto& engine = Engine::GetInstance();
    auto* scene = engine.scene.get();
    auto* framebuffer = m_ViewController->GetGameFramebuffer();

    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    uint32_t width = (uint32_t)contentRegion.x;
    uint32_t height = (uint32_t)contentRegion.y;

    CameraComponent* mainCamera = nullptr;
    std::shared_ptr<GameObject> mainCameraGO = nullptr;

    // Find main camera
    if (scene)
    {
        for (auto& go : scene->GetGameObjects())
        {
            if (go->GetName() == "MainCamera")
            {
                mainCamera = go->GetComponent<CameraComponent>();
                if (mainCamera)
                {
                    mainCameraGO = go;
                    break;
                }
            }
        }

        if (!mainCamera)
        {
            for (auto& go : scene->GetGameObjects())
            {
                auto* cam = go->GetComponent<CameraComponent>();
                if (cam)
                {
                    mainCamera = cam;
                    mainCameraGO = go;
                    break;
                }
            }
        }
    }

    // Setup camera and render
    if (mainCamera)
    {
        mainCamera->SetViewportSize((float)width, (float)height);
        mainCamera->Update();

        m_ViewController->ResizeFramebufferIfNeeded(framebuffer, width, height);

        auto* renderer = engine.renderer.get();
        if (renderer)
            renderer->RenderToFramebuffer(framebuffer, mainCamera);
    }

    // Render UI
    m_UI->RenderGameView(framebuffer, mainCamera, mainCameraGO.get());
}

void ScenePanel::RenderToolbarControls()
{
    auto* scene = Engine::GetInstance().scene.get();

    m_UI->RenderToolbar(
        [this, scene]() {
            // Play/Stop button callback
            bool isPlaying = GameTime::IsPlaying();
            if (!isPlaying)
            {
                m_ViewController->OnPlayModeStarted();
                scene->SaveInitialSnapshot();
                scene->SetSelected(nullptr);
                GameTime::Play();
            }
            else
            {
                GameTime::Stop();
                scene->RestoreSnapshot();
                m_ViewController->OnPlayModeStopped();
            }
        },
        []() {
            // Pause button callback
            GameTime::Pause();
        },
        []() {
            // Step button callback
            GameTime::StepOneFrame();
        }
    );
}

void ScenePanel::CleanUp()
{
    m_ViewController->Cleanup();
}
