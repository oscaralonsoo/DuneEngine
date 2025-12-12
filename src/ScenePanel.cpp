#include "ScenePanel.h"
#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include <imgui.h>
#include <algorithm>
#include "RendererAPI.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include "ModuleRenderer.h"
#include "EditorCamera.h"
#include <glad/glad.h>
#include "CameraComponent.h"
#include "ModuleWindow.h"

bool ScenePanel::Start()
{
    return true;
}

void ScenePanel::OnImGuiRender()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Scene", nullptr, flags);

    // Toolbar controls at the top
    RenderToolbarControls();
    ImGui::Separator();

    // Scene/Game view tabs
    if (ImGui::BeginTabBar("ViewTabs"))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            m_ShowSceneView = true;
            m_ShowGameView = false;
            RenderSceneView();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Game"))
        {
            m_ShowSceneView = false;
            m_ShowGameView = true;
            RenderGameView();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ScenePanel::RenderSceneView()
{
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    // Reserve space for scene view
    ImGui::BeginChild("SceneView", ImVec2(contentRegion.x, m_SceneViewHeight), true,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Get the scene view rectangle
    ImVec2 sceneViewPos = ImGui::GetCursorScreenPos();
    ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();

    // Render the 3D scene here
    // For now, just show a placeholder
    ImGui::Text("Scene View");
    ImGui::Text("Size: %.0f x %.0f", sceneViewSize.x, sceneViewSize.y);

    // TODO: Integrate actual 3D rendering
    // This would involve:
    // 1. Setting up a framebuffer for off-screen rendering
    // 2. Rendering the scene using EditorCamera
    // 3. Displaying the rendered texture in ImGui

    ImGui::EndChild();
}

void ScenePanel::RenderGameView()
{
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("GameView", ImVec2(contentRegion.x, m_GameViewHeight), true,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 gameViewPos = ImGui::GetCursorScreenPos();
    ImVec2 gameViewSize = ImGui::GetContentRegionAvail();

    // Find the MainCamera or first available camera component
    auto* scene = Engine::GetInstance().scene.get();
    CameraComponent* gameCamera = nullptr;

    if (scene)
    {
        for (auto& go : scene->GetGameObjects())
        {
            if (auto* cam = go->GetComponent<CameraComponent>())
            {
                // Look for a camera named "MainCamera" first
                if (go->GetName() == "MainCamera")
                {
                    gameCamera = cam;
                    break;
                }
                // If no MainCamera found, use the first camera
                if (!gameCamera)
                {
                    gameCamera = cam;
                }
            }
        }
    }

    if (gameCamera)
    {
        ImGui::Text("Game View (from %s)", gameCamera->GetOwner()->GetName().c_str());
        ImGui::Text("Size: %.0f x %.0f", gameViewSize.x, gameViewSize.y);
        ImGui::Text("Game camera rendering not yet implemented");
        ImGui::Text("FOV: %.1f°, Near: %.2f, Far: %.1f",
                   gameCamera->GetFOV(), gameCamera->GetNearClip(), gameCamera->GetFarClip());
    }
    else
    {
        ImGui::Text("Game View");
        ImGui::Text("Size: %.0f x %.0f", gameViewSize.x, gameViewSize.y);
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No camera found in scene!");
        ImGui::Text("Create a GameObject with a CameraComponent");
        ImGui::Text("and name it 'MainCamera' for best results.");
    }

    ImGui::EndChild();
}


void ScenePanel::RenderToolbarControls()
{
    bool isPlaying = GameTime::IsPlaying();
    bool isPaused  = GameTime::IsPaused();

    auto* scene = Engine::GetInstance().scene.get();

    // === PLAY / PAUSE ===
    if (!isPlaying)
    {
        if (ImGui::Button("Play"))
        {
            // Guardar snapshot antes del primer Play
            scene->SaveInitialSnapshot();
            scene->ResetSelecteds();
            GameTime::Play();
        }
    }
    else
    {
        if (ImGui::Button("Pause"))
        {
            GameTime::Pause();
        }
    }

    ImGui::SameLine();

    // === STOP ===
    if (ImGui::Button("Stop"))
    {
        GameTime::Stop();
        scene->RestoreSnapshot();
    }

    ImGui::SameLine();

    // === STEP ===
    ImGui::BeginDisabled(!isPaused);
    if (ImGui::Button("Step"))
    {
        GameTime::StepOneFrame();
    }
    ImGui::EndDisabled();

    ImGui::SameLine(0.0f, 30.0f);

    // TimeScale
    float timeScale = GameTime::GetTimeScale();
    ImGui::Text("Speed");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##TimeScale", &timeScale, 0.0f, 4.0f, "%.2f"))
    {
        GameTime::SetTimeScale(timeScale);
    }

    ImGui::SameLine(0.0f, 30.0f);
    ImGui::Text("GameTime: %.2f", GameTime::GetGameTime());
    ImGui::SameLine();
    ImGui::Text("RealTime: %.2f", GameTime::GetRealTimeSinceStartup());
}

void ScenePanel::CleanUp()
{
}
