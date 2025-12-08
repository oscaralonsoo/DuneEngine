#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ResourceUtils.h"
#include "Model.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "PrimitiveMesh.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>

bool HierarchyPanel::IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor)
{
    if (!potentialDescendant || !ancestor) return false;
    auto descendants = ancestor->GetAllDescendants();
    return std::find(descendants.begin(), descendants.end(), potentialDescendant) != descendants.end();
}

bool HierarchyPanel::Start()
{
    return true;
}

void HierarchyPanel::OnImGuiRender()
{
    std::shared_ptr<GameObject> selected = nullptr;

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

    if (auto scene = Engine::GetInstance().scene)
    {
        selected = scene->GetSelected();
        RenderHierarchyTree(scene.get(), selected);
        HandleHierarchyContextMenu(scene.get());
        HandleDragDrop(scene.get());
    }

    ImGui::End();

    ProcessPendingParentChanges();
}

void HierarchyPanel::RenderGameObjectTree(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene)
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

    if (isEditing && editingObject)
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

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("GAMEOBJECT_PAYLOAD", &gameObject, sizeof(std::shared_ptr<GameObject>));
            ImGui::Text("Dragging %s", gameObject->GetName().c_str());
            ImGui::EndDragDropSource();
        }

        // Drop target
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
            {
                std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
                if (dragged != gameObject && !IsDescendant(gameObject, dragged))
                {
                    editingObject = nullptr; // Cancel any ongoing rename to prevent crashes
                    pendingParentChanges.push_back({dragged, gameObject});
                }
            }
            ImGui::EndDragDropTarget();
        }

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
            if (ImGui::MenuItem("Duplicate"))
            {
                editingObject = nullptr; // Cancel any ongoing rename to prevent crashes
                // Duplicate the GameObject
                scene->DuplicateGameObject(gameObject, gameObject->GetParent());
            }
            if (ImGui::MenuItem("Unparent"))
            {
                editingObject = nullptr; // Cancel any ongoing rename to prevent crashes
                pendingParentChanges.push_back({gameObject, nullptr});
            }
            if (ImGui::MenuItem("Delete"))
            {
                editingObject = nullptr; // Cancel any ongoing rename to prevent crashes
                Engine::GetInstance().scene->RemoveGameObject(gameObject);
            }
            ImGui::EndPopup();
        }

        if (nodeOpen)
        {
            auto children = gameObject->GetChildren(); // Copy to avoid iterator invalidation
            for (auto& child : children)
            {
                RenderGameObjectTree(child, selected, scene);
            }
            ImGui::TreePop();
        }
    }
}

void HierarchyPanel::RenderHierarchyTree(ModuleScene* scene, std::shared_ptr<GameObject> selected)
{
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    for (std::shared_ptr<GameObject> gameObject : scene->GetGameObjects())
    {
        // Only render root objects
        if (!gameObject->GetParent())
        {
            RenderGameObjectTree(gameObject, selected, scene);
        }
    }

    // Invisible button to cover the remaining area for drop target
    ImGui::SetCursorPos(ImVec2(0, ImGui::GetCursorPosY()));
    ImGui::InvisibleButton("HierarchyDropTarget", ImVec2(contentSize.x, ImGui::GetContentRegionAvail().y));
}

void HierarchyPanel::HandleHierarchyContextMenu(ModuleScene* scene)
{
    // Right-click context menu for hierarchy background
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("HierarchyContextMenu");
    }

    if (ImGui::BeginPopup("HierarchyContextMenu"))
    {
        if (ImGui::MenuItem("Create Empty GameObject"))
        {
            scene->CreateEmptyGameObject();
        }
        if (ImGui::MenuItem("Create Cube"))
        {
            scene->CreateCube();
        }
        ImGui::EndPopup();
    }
}

void HierarchyPanel::HandleDragDrop(ModuleScene* scene)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            pendingParentChanges.push_back({dragged, nullptr});
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            std::string relativePath = (const char*)payload->Data;
            std::filesystem::path assetPath = "Assets/" + relativePath;
            ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
            if (type == ResourceType::Model)
            {
                scene->CreateGameObjectFromModel(assetPath);
            }
            // Add other types if needed
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyPanel::ProcessPendingParentChanges()
{
    for (const auto& change : pendingParentChanges)
    {
        change.gameObject->SetParent(change.newParent);
    }
    pendingParentChanges.clear();
}

void HierarchyPanel::CleanUp()
{
}
