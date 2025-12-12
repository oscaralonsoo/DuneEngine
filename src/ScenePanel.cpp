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
#include "Framebuffer.h"

bool ScenePanel::Start()
{
    // Framebuffers independientes para Scene y Game
    m_SceneFramebuffer = new Framebuffer(800, 600);
    m_GameFramebuffer  = new Framebuffer(800, 600);
    return true;
}

void ScenePanel::OnImGuiRender()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Scene", nullptr, flags);

    RenderToolbarControls();
    ImGui::Separator();

    if (ImGui::BeginTabBar("ViewTabs"))
    {
        // Scene View → Editor Camera siempre
        if (ImGui::BeginTabItem("Scene"))
        {
            m_ShowSceneView = true;
            m_ShowGameView  = false;
            RenderSceneView();
            ImGui::EndTabItem();
        }

        // Game View → MainCamera siempre
        if (ImGui::BeginTabItem("Game"))
        {
            m_ShowSceneView = false;
            m_ShowGameView  = true;
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

    ImGui::BeginChild("SceneView", contentRegion, true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
    uint32_t width  = (uint32_t)sceneViewSize.x;
    uint32_t height = (uint32_t)sceneViewSize.y;

    if (width > 0 && height > 0 &&
        (m_SceneFramebuffer->GetWidth() != width || m_SceneFramebuffer->GetHeight() != height))
    {
        m_SceneFramebuffer->Resize(width, height);
    }

    auto* renderer = Engine::GetInstance().renderer.get();
    if (renderer && renderer->editorCamera)
        renderer->RenderToFramebuffer(m_SceneFramebuffer, renderer->editorCamera);

    ImGui::Image((ImTextureID)(uintptr_t)m_SceneFramebuffer->GetColorAttachment(),
                 sceneViewSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::EndChild();
}

void ScenePanel::RenderGameView()
{
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("GameView", contentRegion, true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 viewSize = ImGui::GetContentRegionAvail();
    uint32_t width  = (uint32_t)viewSize.x;
    uint32_t height = (uint32_t)viewSize.y;

    // Buscar MainCamera siempre
    auto* scene = Engine::GetInstance().scene.get();
    CameraComponent* mainCamera = nullptr;

    if (scene)
    {
        for (auto& go : scene->GetGameObjects())
        {
            if (go->GetName() == "MainCamera")
            {
                mainCamera = go->GetComponent<CameraComponent>();
                break;
            }
        }
    }

    if (mainCamera)
    {
        mainCamera->Update();

        if (width > 0 && height > 0 &&
            (m_GameFramebuffer->GetWidth() != width || m_GameFramebuffer->GetHeight() != height))
        {
            m_GameFramebuffer->Resize(width, height);
        }

        auto* renderer = Engine::GetInstance().renderer.get();
        if (renderer)
            renderer->RenderToFramebuffer(m_GameFramebuffer, mainCamera);

        ImGui::Image((ImTextureID)(uintptr_t)m_GameFramebuffer->GetColorAttachment(),
                     viewSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        ImGui::Text("Game View");
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "No MainCamera found!");
        ImGui::Text("Create a GameObject named 'MainCamera' with a CameraComponent.");
    }

    ImGui::EndChild();
}

void ScenePanel::RenderToolbarControls()
{
    bool isPlaying = GameTime::IsPlaying();
    bool isPaused  = GameTime::IsPaused();

    auto* scene = Engine::GetInstance().scene.get();

    if (ImGui::Button(isPlaying ? "Stop" : "Play"))
    {
        if (!isPlaying)
        {
            scene->SaveInitialSnapshot();
            GameTime::Play();
        }
        else
        {
            GameTime::Stop();
            scene->RestoreSnapshot();
        }
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!isPlaying);
    if (ImGui::Button("Pause"))
        GameTime::Pause();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!isPaused);
    if (ImGui::Button("Step"))
        GameTime::StepOneFrame();
    ImGui::EndDisabled();

    ImGui::SameLine(0.0f, 30.0f);

    float timeScale = GameTime::GetTimeScale();
    ImGui::Text("Speed");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##TimeScale", &timeScale, 0.0f, 4.0f, "%.2f"))
        GameTime::SetTimeScale(timeScale);

    ImGui::SameLine(0.0f, 30.0f);
    ImGui::Text("GameTime: %.2f", GameTime::GetGameTime());
    ImGui::SameLine();
    ImGui::Text("RealTime: %.2f", GameTime::GetRealTimeSinceStartup());
}

void ScenePanel::CleanUp()
{
    if (m_SceneFramebuffer)
    {
        delete m_SceneFramebuffer;
        m_SceneFramebuffer = nullptr;
    }
    if (m_GameFramebuffer)
    {
        delete m_GameFramebuffer;
        m_GameFramebuffer = nullptr;
    }
}
