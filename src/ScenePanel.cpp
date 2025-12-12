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
    
    bool isPlaying = GameTime::IsPlaying();
    
    if (ImGui::BeginTabBar("ViewTabs"))
    {
        //-------Scene Tab--------
        ImGuiTabItemFlags sceneFlags = 0;
        
        if (m_ForceRestoreView && m_CurrentView == ViewType::Scene)
        sceneFlags |= ImGuiTabItemFlags_SetSelected;
        
        ImGui::BeginDisabled(isPlaying);
        if (ImGui::BeginTabItem("Scene", nullptr, sceneFlags))
        {
            RenderSceneView();
            
            if (!isPlaying && ImGui::IsItemActivated())
            m_CurrentView = ViewType::Scene;
            
            ImGui::EndTabItem();
        }
        ImGui::EndDisabled();
        
        //--------Game Tab--------
        ImGuiTabItemFlags gameFlags = 0;
        
        if (isPlaying || (m_ForceRestoreView && m_CurrentView == ViewType::Game))
        gameFlags |= ImGuiTabItemFlags_SetSelected;
        
        if (ImGui::BeginTabItem("Game", nullptr, gameFlags))
        {
            RenderGameView();
            
            if (ImGui::IsItemActivated())
            m_CurrentView = ViewType::Game;
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
        
        m_ForceRestoreView = false;
    }
    ImGui::End();
}
void ScenePanel::RenderSceneView()
{
    const bool sceneEditable = IsSceneEditable();

    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("SceneView", contentRegion, true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
    uint32_t width  = (uint32_t)sceneViewSize.x;
    uint32_t height = (uint32_t)sceneViewSize.y;

    if (width > 0 && height > 0 &&
        (m_SceneFramebuffer->GetWidth() != width ||
         m_SceneFramebuffer->GetHeight() != height))
    {
        m_SceneFramebuffer->Resize(width, height);
    }

    //Render
    auto* renderer = Engine::GetInstance().renderer.get();
    if (renderer && renderer->editorCamera)
    {
        renderer->editorCamera->SetInputEnabled(sceneEditable);
        renderer->RenderToFramebuffer(m_SceneFramebuffer, renderer->editorCamera);
    }

    ImGui::Image(
        (ImTextureID)(uintptr_t)m_SceneFramebuffer->GetColorAttachment(),
        sceneViewSize, ImVec2(0, 1), ImVec2(1, 0));

    //Input blocked
    if (!sceneEditable)
    {
        ImGui::EndChild();
        return;
    }

    //Selection
    if (ImGui::IsItemClicked(0))
    {
        auto* scene = Engine::GetInstance().scene.get();
        std::shared_ptr<GameObject> picked =
            scene->GetRaycaster()->PickObject(
                ImGui::GetMousePos().x - ImGui::GetItemRectMin().x,
                ImGui::GetMousePos().y - ImGui::GetItemRectMin().y,
                width, height,
                scene->GetGameObjects());

        scene->SetSelected(picked);
    }

    // Drag & Drop
    float mouseX = -1.0f, mouseY = -1.0f;
    if (ImGui::IsItemHovered())
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemMin  = ImGui::GetItemRectMin();
        mouseX = mousePos.x - itemMin.x;
        mouseY = mousePos.y - itemMin.y;
    }

    HandleSceneDragDrop(sceneViewSize, mouseX, mouseY);

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
    
    auto* scene = Engine::GetInstance().scene.get();
    CameraComponent* mainCamera = nullptr;
    std::shared_ptr<GameObject> mainCameraGO = nullptr;

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

    if (mainCamera)
    {
        mainCamera->SetViewportSize((float)width, (float)height);
        mainCamera->Update();                    // Genera ViewMatrix + posición

        // Resize framebuffer si hace falta
        if (width > 0 && height > 0 &&
            (m_GameFramebuffer->GetWidth() != width || m_GameFramebuffer->GetHeight() != height))
        {
            m_GameFramebuffer->Resize(width, height);
        }

        // Render
        auto* renderer = Engine::GetInstance().renderer.get();
        if (renderer)
            renderer->RenderToFramebuffer(m_GameFramebuffer, mainCamera);

        // Mostrar textura
        ImGui::Image((ImTextureID)(uintptr_t)m_GameFramebuffer->GetColorAttachment(),
                     viewSize, ImVec2(0, 1), ImVec2(1, 0));

        glm::vec3 camPos = mainCamera->GetPosition();
        std::string camName = mainCameraGO ? mainCameraGO->GetName() : std::string("Unnamed Camera");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 18.0f); // superponer texto arriba del frame
        ImGui::TextColored(ImVec4(0.9f,0.9f,0.9f,0.9f), "Cam: %s (%.2f, %.2f, %.2f)", camName.c_str(),
                           camPos.x, camPos.y, camPos.z);
    }
    else
    {
        ImGui::Text("Game View");
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "No camera found in the scene!");
        ImGui::Text("Create a GameObject with a CameraComponent or name one 'MainCamera'.");
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
            // Guardar la vista actual
            m_LastViewBeforePlay = m_CurrentView;

            // Forzar Game View
            m_CurrentView = ViewType::Game;

            scene->SaveInitialSnapshot();
            GameTime::Play();
        }
        else
        {
            GameTime::Stop();
            scene->RestoreSnapshot();
            
            m_CurrentView = m_LastViewBeforePlay;
            m_ForceRestoreView = true; // ← CLAVE
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
bool ScenePanel::IsSceneEditable() const
{
    return !GameTime::IsPlaying() &&
           m_CurrentView == ViewType::Scene;
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