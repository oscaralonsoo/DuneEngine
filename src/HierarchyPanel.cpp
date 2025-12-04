#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ResourceUtils.h"
#include "Model.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>

bool HierarchyPanel::Start()
{
    return true;
}

void HierarchyPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredWidth = workSize.x * HierarchyPanel::kDefaultFraction;
    float otherPanelEstimate = workSize.x * HierarchyPanel::kDefaultFraction;
    float maxAllowed = workSize.x - otherPanelEstimate - HierarchyPanel::kMinCenterWidth;
    float panelWidth = std::clamp(desiredWidth, HierarchyPanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(workPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;


    ImGui::Begin("Hierarchy", nullptr, flags);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    if (auto scene = Engine::GetInstance().scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        for (std::shared_ptr<GameObject> gameObject : scene->GetGameObjects())
        {
            // Only render root objects
            if (!gameObject->GetParent())
            {
                RenderGameObjectTree(gameObject, selected);
            }
        }
    }

    // Invisible button to cover the remaining area for drop target
    ImGui::SetCursorPos(ImVec2(0, ImGui::GetCursorPosY()));
    ImGui::InvisibleButton("HierarchyDropTarget", ImVec2(contentSize.x, ImGui::GetContentRegionAvail().y));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            std::string relativePath = (const char*)payload->Data;
            std::filesystem::path assetPath = "Assets/" + relativePath;
            ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
            auto scene = Engine::GetInstance().scene;
            if (scene)
            {
                auto gameObject = scene->CreateGameObject();
                std::string baseName = assetPath.stem().string();
                std::string uniqueName = baseName;
                int counter = 1;
                while (true)
                {
                    bool nameExists = false;
                    for (auto& existingGameObject : scene->GetGameObjects())
                    {
                        if (existingGameObject->GetName() == uniqueName)
                        {
                            nameExists = true;
                            break;
                        }
                    }
                    if (!nameExists)
                        break;
                    uniqueName = baseName + "_" + std::to_string(counter);
                    counter++;
                }
                gameObject->SetName(uniqueName);
                gameObject->CreateComponent(ComponentType::Transform);
                if (type == ResourceType::Model)
                {
                    auto model = Model::Load(assetPath);
                    if (model && !model->GetMeshes().empty())
                    {
                        if (model->GetMeshes().size() > 1)
                        {
                            for (size_t i = 0; i < model->GetMeshes().size(); ++i)
                            {
                                auto childGo = scene->CreateGameObject();
                                std::string childName = uniqueName + "_" + std::to_string(i);
                                childGo->SetName(childName);
                                childGo->CreateComponent(ComponentType::Transform);
                                childGo->CreateComponent(ComponentType::Mesh);
                                childGo->GetComponent<MeshComponent>()->SetMesh(model->GetMeshes()[i]);
                                childGo->CreateComponent(ComponentType::Material);
                                childGo->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                                childGo->SetParent(gameObject);
                            }
                        }
                        else
                        {
                            // Single mesh
                            gameObject->CreateComponent(ComponentType::Mesh);
                            gameObject->GetComponent<MeshComponent>()->SetMesh(model->GetMeshes()[0]);
                            gameObject->CreateComponent(ComponentType::Material);
                            gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                        }
                    }
                }
                // Add other types if needed
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

void HierarchyPanel::RenderGameObjectTree(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected)
{
    static std::shared_ptr<GameObject> editingObject = nullptr;
    static char editBuffer[256];

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (gameObject == selected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool hasChildren = !gameObject->GetChildren().empty();
    if (!hasChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool isEditing = (editingObject == gameObject);

    if (isEditing)
    {
        strcpy(editBuffer, gameObject->GetName().c_str());
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##edit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            gameObject->SetName(editBuffer);
            editingObject = nullptr;
        }
        if (ImGui::IsItemDeactivated())
        {
            editingObject = nullptr;
        }
    }
    else
    {
        bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags);

        if (ImGui::IsItemClicked())
        {
            Engine::GetInstance().scene->SetSelected(gameObject);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            editingObject = gameObject;
        }

        // Right-click context menu
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup("GameObjectContextMenu");
        }

        if (ImGui::BeginPopup("GameObjectContextMenu"))
        {
            if (ImGui::MenuItem("Rename"))
            {
                editingObject = gameObject;
            }
            if (ImGui::MenuItem("Delete"))
            {
                Engine::GetInstance().scene->RemoveGameObject(gameObject);
            }
            ImGui::EndPopup();
        }

        if (nodeOpen)
        {
            for (auto& child : gameObject->GetChildren())
            {
                RenderGameObjectTree(child, selected);
            }
            ImGui::TreePop();
        }
    }
}

void HierarchyPanel::CleanUp()
{
}
