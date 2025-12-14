#include "HierarchyDragDrop.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ResourceUtils.h"
#include <imgui.h>
#include <filesystem>
#include <algorithm>

HierarchyDragDrop::HierarchyDragDrop()
{
}

bool HierarchyDragDrop::IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor)
{
    if (!potentialDescendant || !ancestor) return false;
    auto descendants = ancestor->GetAllDescendants();
    return std::find(descendants.begin(), descendants.end(), potentialDescendant) != descendants.end();
}

void HierarchyDragDrop::HandleDragSource(std::shared_ptr<GameObject> gameObject)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("GAMEOBJECT_PAYLOAD", &gameObject, sizeof(std::shared_ptr<GameObject>));
        ImGui::Text("Dragging: %s", gameObject->GetName().c_str());
        ImGui::EndDragDropSource();
    }
}

void HierarchyDragDrop::HandleNodeDropTarget(std::shared_ptr<GameObject> targetGameObject, ModuleScene* scene)
{
    if (ImGui::BeginDragDropTarget())
    {
        // Handle GameObject drop
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            HandleGameObjectDrop(dragged, targetGameObject);
        }
        // Handle asset drop
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            std::string relativePath = (const char*)payload->Data;
            HandleAssetDrop(relativePath.c_str(), targetGameObject, scene);
        }
        // Handle external file drop
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILES"))
        {
            const char* files = (const char*)payload->Data;
            HandleFileDrop(files, targetGameObject, scene);
        }
        
        ImGui::EndDragDropTarget();
    }
}

void HierarchyDragDrop::HandleBackgroundDropTarget(ModuleScene* scene)
{
    if (ImGui::BeginDragDropTarget())
    {
        // Handle GameObject drop
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            pendingParentChanges.push_back({dragged, nullptr, -1});
        }
        // Handle asset drop
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            std::string relativePath = (const char*)payload->Data;
            HandleAssetDrop(relativePath.c_str(), nullptr, scene);
        }
        
        ImGui::EndDragDropTarget();
    }
}

void HierarchyDragDrop::HandleGameObjectDrop(std::shared_ptr<GameObject> dragged, std::shared_ptr<GameObject> target)
{
    if (dragged != target && !IsDescendant(target, dragged))
    {
        pendingParentChanges.push_back({dragged, target, -1});
    }
}

void HierarchyDragDrop::HandleDropBetweenNodes(std::shared_ptr<GameObject> afterGameObject, ModuleScene* scene)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PAYLOAD"))
        {
            std::shared_ptr<GameObject> dragged = *(std::shared_ptr<GameObject>*)payload->Data;
            
            if (dragged != afterGameObject)
            {
                auto parent = afterGameObject->GetParent();
                
                // Don't allow dropping a parent into its own descendants
                if (!IsDescendant(afterGameObject, dragged))
                {
                    if (parent)
                    {
                        size_t targetIndex = parent->GetChildIndex(afterGameObject);
                        if (targetIndex != static_cast<size_t>(-1))
                        {
                            if (dragged->GetParent() == parent)
                            {
                                size_t currentIndex = parent->GetChildIndex(dragged);
                                if (currentIndex < targetIndex)
                                {
                                    targetIndex--;
                                }
                            }
                            pendingParentChanges.push_back({dragged, parent, static_cast<int>(targetIndex + 1)});
                        }
                    }
                    else
                    {
                        // Root level reordering
                        pendingParentChanges.push_back({dragged, nullptr, -1});
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyDragDrop::HandleAssetDrop(const char* relativePath, std::shared_ptr<GameObject> parent, ModuleScene* scene)
{
    std::filesystem::path assetPath = "Assets/" + std::string(relativePath);
    ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
    
    if (type == ResourceType::Model)
    {
        auto newGameObject = scene->CreateGameObjectFromModel(assetPath);
        if (parent)
        {
            newGameObject->SetParent(parent);
        }
        scene->SetSelected(newGameObject);
    }
}

void HierarchyDragDrop::HandleFileDrop(const char* files, std::shared_ptr<GameObject> parent, ModuleScene* scene)
{
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
                if (parent)
                {
                    newGameObject->SetParent(parent);
                }
                scene->SetSelected(newGameObject);
            }
        }
        start += filePath.size() + 1;
    }
}

void HierarchyDragDrop::ProcessPendingChanges()
{
    for (const auto& change : pendingParentChanges)
    {
        if (change.insertIndex >= 0)
        {
            if (change.newParent)
            {
                change.newParent->InsertChildAt(change.gameObject, static_cast<size_t>(change.insertIndex));
            }
        }
        else
        {
            change.gameObject->SetParent(change.newParent);
        }
    }
    pendingParentChanges.clear();
}
