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
#include <string>

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

void HierarchyPanel::SetupWindow(ImGuiViewport* viewport, float& panelWidth)
{
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredWidth = workSize.x * HierarchyPanel::kDefaultFraction;
    float otherPanelEstimate = workSize.x * HierarchyPanel::kDefaultFraction;
    float maxAllowed = workSize.x - otherPanelEstimate - HierarchyPanel::kMinCenterWidth;
    panelWidth = std::clamp(desiredWidth, HierarchyPanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(workPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(viewport->ID);
}

void HierarchyPanel::RenderContent(ModuleScene* scene, std::shared_ptr<GameObject> selected)
{
    RenderHierarchyTree(scene, selected);
    HandleHierarchyContextMenu(scene);
}

void HierarchyPanel::OnImGuiRender()
{
    std::shared_ptr<GameObject> selected = nullptr;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Hierarchy", nullptr, flags);

    if (auto scene = Engine::GetInstance().scene)
    {
        selected = scene->GetSelected();
        RenderHierarchyTree(scene.get(), selected);
        HandleHierarchyContextMenu(scene.get());
    }

    ImGui::End();

    ProcessPendingParentChanges();
}

void HierarchyPanel::RenderGameObjectTree(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene)
{
    // Push unique ID
    ImGui::PushID(gameObject.get());

    if (editingObject == gameObject)
    {
        RenderTreeNode(gameObject, selected, scene);
        ImGui::PopID();
        return;
    }

    bool hasChildren = !gameObject->GetChildren().empty();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (gameObject == selected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!hasChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags);
    

    // Handle drag/drop 
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("GAMEOBJECT_PAYLOAD", &gameObject, sizeof(std::shared_ptr<GameObject>));
        ImGui::Text("Dragging: %s", gameObject->GetName().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            if (dragged != gameObject && !IsDescendant(gameObject, dragged))
            {
                editingObject = nullptr;
                pendingParentChanges.push_back({dragged, gameObject});
            }
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            std::string relativePath = (const char*)payload->Data;
            std::filesystem::path assetPath = "Assets/" + relativePath;
            ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
            if (type == ResourceType::Model)
            {
                auto newGameObject = scene->CreateGameObjectFromModel(assetPath);
                newGameObject->SetParent(gameObject);
                scene->SetSelected(newGameObject);
            }
            // Add other types if needed
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILES"))
        {
            const char* files = (const char*)payload->Data;
            size_t len = strlen(files);
            size_t start = 0;
            while (start < len)
            {
                std::string filePath = &files[start];
                if (!filePath.empty())
                {
                    std::filesystem::path path(filePath);
                    ResourceType type = ResourceUtils::GetTypeFromExtension(path);
                    if (type == ResourceType::Model)
                    {
                        auto newGameObject = scene->CreateGameObjectFromModel(path);
                        newGameObject->SetParent(gameObject);
                        scene->SetSelected(newGameObject);
                    }
                }
                start += filePath.size() + 1;
            }
        }
        ImGui::EndDragDropTarget();
    }

    HandleNodeInteraction(gameObject, scene);
    RenderGameObjectContextMenu(gameObject, scene);

    if (nodeOpen)
    {
        if (hasChildren)
        {
            auto children = gameObject->GetChildren();
            for (auto& child : children)
            {
                RenderGameObjectTree(child, selected, scene);
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void HierarchyPanel::RenderTreeNode(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene)
{
    if (editingObject == gameObject)
    {
        strcpy(editBuffer, gameObject->GetName().c_str());
        ImGui::SetNextItemWidth(-1);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##edit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            gameObject->SetName(editBuffer);
            editingObject = nullptr;
        }
        if (ImGui::IsItemDeactivated())
        {
            editingObject = nullptr;
        }
        return;
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("GAMEOBJECT_PAYLOAD", &gameObject, sizeof(std::shared_ptr<GameObject>));
        ImGui::Text("Dragging: %s", gameObject->GetName().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            if (dragged != gameObject && !IsDescendant(gameObject, dragged))
            {
                editingObject = nullptr;
                pendingParentChanges.push_back({dragged, gameObject});
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyPanel::HandleNodeInteraction(std::shared_ptr<GameObject> gameObject, ModuleScene* scene)
{
    if (ImGui::IsItemClicked())
    {
        Engine::GetInstance().scene->SetSelected(gameObject);
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        editingObject = gameObject;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("GameObjectContextMenu");
    }
}

void HierarchyPanel::RenderGameObjectContextMenu(std::shared_ptr<GameObject> gameObject, ModuleScene* scene)
{
    if (ImGui::BeginPopup("GameObjectContextMenu"))
    {
        if (ImGui::MenuItem("Rename"))
        {
            editingObject = gameObject;
        }
        if (ImGui::MenuItem("Duplicate"))
        {
            editingObject = nullptr;
            scene->DuplicateGameObject(gameObject, gameObject->GetParent());
        }
        if (ImGui::BeginMenu("Create Child"))
        {
            if (ImGui::MenuItem("Empty"))
            {
                editingObject = nullptr;
                auto child = scene->CreateEmptyGameObject();
                child->SetParent(gameObject);
            }
            
            ImGui::Separator();
            
            if (ImGui::BeginMenu("3D Object"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateCube();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Sphere"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateSphere();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Capsule"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateCapsule();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Cylinder"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateCylinder();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Cone"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateCone();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Plane"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreatePlane();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Quad"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateQuad();
                    child->SetParent(gameObject);
                }
                if (ImGui::MenuItem("Torus"))
                {
                    editingObject = nullptr;
                    auto child = scene->CreateTorus();
                    child->SetParent(gameObject);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Camera"))
            {
                editingObject = nullptr;
                auto child = scene->CreateCamera();
                child->SetParent(gameObject);
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Unparent"))
        {
            editingObject = nullptr;
            pendingParentChanges.push_back({gameObject, nullptr});
        }
        if (ImGui::MenuItem("Delete"))
        {
            editingObject = nullptr;
            Engine::GetInstance().scene->RemoveGameObject(gameObject);
        }
        ImGui::EndPopup();
    }
}

void HierarchyPanel::RenderHierarchyTree(ModuleScene* scene, std::shared_ptr<GameObject> selected)
{
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    for (std::shared_ptr<GameObject> gameObject : scene->GetGameObjects())
    {
        if (!gameObject->GetParent())
        {
            RenderGameObjectTree(gameObject, selected, scene);
        }
    }

    ImGui::SetCursorPos(ImVec2(0, ImGui::GetCursorPosY()));
    ImGui::InvisibleButton("HierarchyDropTarget", ImVec2(contentSize.x, ImGui::GetContentRegionAvail().y));

    // Handle deselection on click away
    if (ImGui::IsItemClicked() && selected)
    {
        scene->SetSelected(nullptr);
    }
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
        
        ImGui::Separator();
        
        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                scene->CreateCube();
            }
            if (ImGui::MenuItem("Sphere"))
            {
                scene->CreateSphere();
            }
            if (ImGui::MenuItem("Capsule"))
            {
                scene->CreateCapsule();
            }
            if (ImGui::MenuItem("Cylinder"))
            {
                scene->CreateCylinder();
            }
            if (ImGui::MenuItem("Cone"))
            {
                scene->CreateCone();
            }
            if (ImGui::MenuItem("Plane"))
            {
                scene->CreatePlane();
            }
            if (ImGui::MenuItem("Quad"))
            {
                scene->CreateQuad();
            }
            if (ImGui::MenuItem("Torus"))
            {
                scene->CreateTorus();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem("Camera"))
        {
            scene->CreateCamera();
        }
        
        ImGui::EndPopup();
    }

    // Handle drag and drop on the hierarchy background
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
                auto gameObject = scene->CreateGameObjectFromModel(assetPath);
                scene->SetSelected(gameObject);
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
