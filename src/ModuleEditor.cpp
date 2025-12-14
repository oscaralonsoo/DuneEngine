#include "Engine.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include "ResourceUtils.h"
#include "Model.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "ModuleInput.h"
#include "ModuleResource.h"
#include "Texture.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <cstring>
#include "GameTime.h"
#include "ScenePanel.h"

ModuleEditor::ModuleEditor()
{
    name = "editor";
}

bool ModuleEditor::Start()
{
    AddPanel<HierarchyPanel>();
    AddPanel<InspectorPanel>();
    AddPanel<ProjectPanel>();
    AddPanel<ScenePanel>();

    for (auto& panel : panels)
        panel->Start();
    
    return true;
}

bool ModuleEditor::Update()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    // Ventana invisible a pantalla completa que aloja el DockSpace
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoBackground; // deja ver tu escena
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DockSpaceHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    // Crea el DockSpace
    ImGuiID dockspace_id = ImGui::GetID("DuneEngineDockSpace");
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, ImVec2(0,0), dock_flags);
    ImGui::End();

    for (auto& panel : panels)
    {
        if (Panel* p = dynamic_cast<Panel*>(panel.get()))
            p->Render();
        else
            panel->OnImGuiRender();
    }

    // Handle file drops that weren't processed by ProjectPanel
    // This runs after all panels have rendered, so ProjectPanel had a chance to handle it
    ModuleInput* input = Engine::GetInstance().input.get();
    if (input && input->WasFileDropped())
    {
        const std::string& file = input->GetDraggedFile();
        if (!file.empty())
        {
            // If we reach here, ProjectPanel didn't handle the drop
            // So we handle it for the scene (create GameObjects)
            ResourceType type = ResourceUtils::GetTypeFromExtension(file);

            if (type == ResourceType::Model)
            {
                std::shared_ptr<Model> model = std::make_shared<Model>(file);
                std::string baseName = std::filesystem::path(file).filename().stem().string();

                // Create parent
                std::shared_ptr<GameObject> parentGameObject = Engine::GetInstance().scene.get()->CreateGameObjectWithName(baseName);

                auto* scene = Engine::GetInstance().scene.get();

                for (size_t i = 0; i < model->GetMeshes().size(); ++i)
                {
                    std::shared_ptr<GameObject> childGameObject = scene->CreateGameObject();
                    if (model->GetMeshes().size() > 1)
                    {
                        childGameObject->SetName(baseName + "_" + std::to_string(i));
                    }
                    else
                    {
                        childGameObject->SetName(baseName);
                    }

                    auto &mesh = model->GetMeshes()[i];

                    childGameObject->CreateComponent(ComponentType::Transform);
                    childGameObject->CreateComponent(ComponentType::Mesh);
                    MeshComponent *meshComp = childGameObject->GetComponent<MeshComponent>();
                    meshComp->SetMesh(mesh);

                    childGameObject->CreateComponent(ComponentType::Material);
                    MaterialComponent *materialComponent = childGameObject->GetComponent<MaterialComponent>();
                    materialComponent->SetMaterial(std::make_shared<Material>(ResourceType::Material));

                    // Set parent-child relation
                    childGameObject->SetParent(parentGameObject);

                    scene->AddGameObject(childGameObject);
                }

                Engine::GetInstance().scene->RebuildQuadtree();
            }
            else if (type == ResourceType::Texture)
            {
                ModuleScene *scene = Engine::GetInstance().scene.get();
                std::shared_ptr<GameObject> selected = scene->GetSelected();

                if (selected)
                {
                    MaterialComponent *materialComponent = selected->GetComponent<MaterialComponent>();
                    if (materialComponent)
                    {
                        std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;
                        std::shared_ptr<Resource> resource = resourceManager->RequestResource(file);
                        std::shared_ptr<Texture> texture = std::dynamic_pointer_cast<Texture>(resource);
                        materialComponent->GetMaterial()->SetTexture(TextureType::Albedo, texture);
                    }
                }
            }

            // Clear the drop state since we handled it
            input->ClearDraggedFile();
            input->ClearDropState();
        }
    }

    return true;
}

bool ModuleEditor::CleanUp()
{
    for (auto& panel : panels)
    {
        panel->CleanUp();
    }
    panels.clear();
    return true;
}
