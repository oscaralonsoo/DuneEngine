#include "ScenePanelUI.h"
#include "SceneViewController.h"
#include "GameTime.h"
#include "Engine.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Gizmo.h"
#include "Renderer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

void ScenePanelUI::RenderToolbar(
    std::function<void()> onPlayStop,
    std::function<void()> onPause,
    std::function<void()> onStep)
{
    bool isPlaying = GameTime::IsPlaying();
    bool isPaused = GameTime::IsPaused();

    if (ImGui::Button(isPlaying ? "Stop" : "Play"))
    {
        if (onPlayStop)
            onPlayStop();
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!isPlaying);
    if (ImGui::Button("Pause"))
    {
        if (onPause)
            onPause();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!isPaused);
    if (ImGui::Button("Step"))
    {
        if (onStep)
            onStep();
    }
    ImGui::EndDisabled();

    ImGui::SameLine(0.0f, 30.0f);

    // Wireframe toggle button
    bool wireframeEnabled = Renderer::IsWireframeEnabled();
    if (ImGui::Checkbox("Wireframe", &wireframeEnabled))
    {
        Renderer::SetWireframeMode(wireframeEnabled);
    }

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

void ScenePanelUI::RenderSceneView(
    Framebuffer* framebuffer,
    std::function<void(float, float, bool)> onMouseInteraction,
    std::function<void()> onKeyboardInput,
    std::function<void(ImVec2, float, float)> onDragDrop)
{
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("SceneView", contentRegion, true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
    ImVec2 itemMin = ImGui::GetCursorScreenPos();

    // Display framebuffer
    if (framebuffer)
    {
        ImGui::Image(
            (ImTextureID)(uintptr_t)framebuffer->GetColorAttachment(),
            sceneViewSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }

    float mouseX = -1.0f;
    float mouseY = -1.0f;
    bool hovered = ImGui::IsItemHovered();

    // Handle keyboard shortcuts
    if (hovered && !GameTime::IsPlaying() && onKeyboardInput)
    {
        onKeyboardInput();
    }

    // Handle mouse interactions
    if (onMouseInteraction)
    {
        // Mouse click 
        if (hovered && ImGui::IsMouseClicked(0))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            mouseX = mousePos.x - itemMin.x;
            mouseY = mousePos.y - itemMin.y;
            onMouseInteraction(mouseX, mouseY, true);
            m_IsDragging = true;
        }
        else if (m_IsDragging && ImGui::IsMouseDown(0))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            mouseX = mousePos.x - itemMin.x;
            mouseY = mousePos.y - itemMin.y;
            
            onMouseInteraction(mouseX, mouseY, false);
        }
        // Mouse release
        else if (ImGui::IsMouseReleased(0))
        {
            m_IsDragging = false;
            onMouseInteraction(-1.0f, -1.0f, false);
        }
        // Mouse hover 
        else if (hovered && !m_IsDragging)
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            mouseX = mousePos.x - itemMin.x;
            mouseY = mousePos.y - itemMin.y;
            onMouseInteraction(mouseX, mouseY, false);
        }
    }

    // Handle drag and drop
    if (onDragDrop)
    {
        // Get current mouse position for drag & drop
        if (hovered)
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            mouseX = mousePos.x - itemMin.x;
            mouseY = mousePos.y - itemMin.y;
        }
        onDragDrop(sceneViewSize, mouseX, mouseY);
    }

    ImGui::EndChild();
}

void ScenePanelUI::RenderGameView(
    Framebuffer* framebuffer,
    CameraComponent* camera,
    GameObject* cameraGameObject)
{
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("GameView", contentRegion, true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 viewSize = ImGui::GetContentRegionAvail();

    if (camera && framebuffer)
    {
        // Display framebuffer
        ImGui::Image(
            (ImTextureID)(uintptr_t)framebuffer->GetColorAttachment(),
            viewSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        // Display camera info
        glm::vec3 camPos = camera->GetPosition();
        std::string camName = cameraGameObject ? cameraGameObject->GetName() : std::string("Unnamed Camera");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 18.0f);
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 0.9f), "Cam: %s (%.2f, %.2f, %.2f)",
            camName.c_str(), camPos.x, camPos.y, camPos.z);
    }
    else
    {
        ImGui::Text("Game View");
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No camera found in the scene!");
        ImGui::Text("Create a GameObject with a CameraComponent.");
    }

    ImGui::EndChild();
}

void ScenePanelUI::RenderTabBar(
    ViewType currentView,
    bool isPlaying,
    bool forceRestore,
    std::function<void(ViewType)> onViewChanged)
{
    if (ImGui::BeginTabBar("ViewTabs", ImGuiTabBarFlags_None))
    {
        // Scene Tab
        ImGuiTabItemFlags sceneFlags = 0;
        if (forceRestore && currentView == ViewType::Scene)
            sceneFlags |= ImGuiTabItemFlags_SetSelected;

        ImGui::BeginDisabled(isPlaying);
        if (ImGui::BeginTabItem("Scene", nullptr, sceneFlags))
        {
            if (!isPlaying && onViewChanged && currentView != ViewType::Scene)
            {
                onViewChanged(ViewType::Scene);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndDisabled();

        // Game Tab
        ImGuiTabItemFlags gameFlags = 0;
        if (isPlaying || (forceRestore && currentView == ViewType::Game))
            gameFlags |= ImGuiTabItemFlags_SetSelected;

        if (ImGui::BeginTabItem("Game", nullptr, gameFlags))
        {
            if (onViewChanged && currentView != ViewType::Game)
            {
                onViewChanged(ViewType::Game);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
