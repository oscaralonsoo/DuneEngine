#include "ScenePanel.h"
#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ResourceUtils.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
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
#include "PrimitiveMesh.h"

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

    // Handle selection on click
    if (ImGui::IsItemClicked(0))
    {
        auto* scene = Engine::GetInstance().scene.get();
        std::shared_ptr<GameObject> picked = scene->GetRaycaster()->PickObject(
            ImGui::GetMousePos().x - ImGui::GetItemRectMin().x,
            ImGui::GetMousePos().y - ImGui::GetItemRectMin().y,
            width, height,
            scene->GetGameObjects());
        if (picked)
        {
            scene->SetSelected(picked);
        }
        else
        {
            scene->SetSelected(nullptr);
        }
    }

    // Calculate mouse position relative to the scene view for drag and drop
    float mouseX = -1.0f, mouseY = -1.0f;
    if (ImGui::IsItemHovered())
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemMin = ImGui::GetItemRectMin();
        mouseX = mousePos.x - itemMin.x;
        mouseY = mousePos.y - itemMin.y;
    }

    // Handle drag and drop for Scene View only
    HandleSceneDragDrop(sceneViewSize, mouseX, mouseY);

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

void ScenePanel::HandleSceneDragDrop(ImVec2 sceneViewSize, float mouseX, float mouseY)
{
    if (ImGui::BeginDragDropTarget())
    {
        // Handle assets from ProjectPanel
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            const char* relativePath = (const char*)payload->Data;
            if (relativePath && relativePath[0] != '\0')
            {
                std::filesystem::path fullPath = "Assets" / std::filesystem::path(relativePath);
                HandleAssetDrop(fullPath, mouseX, mouseY);
            }
        }
        // Handle external files from File Explorer
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* path = (const char*)payload->Data;
            if (path && path[0] != '\0')
            {
                std::filesystem::path assetPath = path;
                HandleAssetDrop(assetPath, mouseX, mouseY);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Also handle file drops via ModuleInput
    auto* input = Engine::GetInstance().input.get();
    if (input && input->WasFileDropped())
    {
        std::string path = input->GetDraggedFile();
        if (!path.empty())
        {
            HandleAssetDrop(std::filesystem::path(path));
            input->ClearDropState();
            input->ClearDraggedFile();
        }
    }
}

void ScenePanel::HandleAssetDrop(const std::filesystem::path& assetPath, float mouseX, float mouseY)
{
    auto* scene = Engine::GetInstance().scene.get();
    if (!scene) return;

    ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);

    int width = m_SceneFramebuffer->GetWidth();
    int height = m_SceneFramebuffer->GetHeight();

    GameObject* hoveredObject = nullptr;
    if (mouseX >= 0.0f && mouseY >= 0.0f)
    {
        auto picked = scene->GetRaycaster()->PickObject(mouseX, mouseY, width, height, scene->GetGameObjects());
        hoveredObject = picked.get();
    }

    switch (type)
    {
        case ResourceType::Unknown:
            return;

        case ResourceType::Texture:
        {
            if (!hoveredObject)
                return;

            auto* matComp = hoveredObject->GetComponent<MaterialComponent>();
            if (!matComp)
                return;

            auto material = matComp->GetMaterial();
            if (!material)
                return;

            material->GetTextures().albedo = std::make_shared<Texture>(assetPath);
            matComp->SetMaterial(material);
            return;
        }

        case ResourceType::Model:
            scene->CreateGameObjectFromModel(assetPath);
            return;

        case ResourceType::Prefab:
            scene->CreateGameObjectFromPrefab(assetPath);
            return;

        default:
            return;
    }
}



void ScenePanel::CleanUp()
{
}
