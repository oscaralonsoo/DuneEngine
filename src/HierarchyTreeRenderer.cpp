#include "HierarchyTreeRenderer.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "HierarchyDragDrop.h"
#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleEditor.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

HierarchyTreeRenderer::HierarchyTreeRenderer()
    : editingObject(nullptr)
{
    memset(editBuffer, 0, sizeof(editBuffer));
}

void HierarchyTreeRenderer::RenderNode(
    std::shared_ptr<GameObject> gameObject,
    std::shared_ptr<GameObject> selected,
    ModuleScene* scene,
    HierarchyDragDrop* dragDropHandler,
    HierarchyPanel* hierarchyPanel)
{
    ImGui::PushID(gameObject.get());

    if (editingObject == gameObject)
    {
        RenderEditableNode(gameObject, selected, scene);
        ImGui::PopID();
        return;
    }

    ImGuiTreeNodeFlags flags = SetupNodeFlags(gameObject, selected);

    // Render tree
    bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags);

    dragDropHandler->HandleDragSource(gameObject);

    dragDropHandler->HandleNodeDropTarget(gameObject, scene);

    HandleNodeInteraction(gameObject, scene);
    
    hierarchyPanel->RenderNodeContextMenu(gameObject, scene);

    // Render children
    if (nodeOpen)
    {
        bool hasChildren = !gameObject->GetChildren().empty();
        if (hasChildren)
        {
            auto children = gameObject->GetChildren();
            for (auto& child : children)
            {
                RenderNode(child, selected, scene, dragDropHandler, hierarchyPanel);
            }
        }
        ImGui::TreePop();
    }

    RenderDropZone(gameObject, dragDropHandler);

    ImGui::PopID();
}

void HierarchyTreeRenderer::HandleNodeInteraction(std::shared_ptr<GameObject> gameObject, ModuleScene* scene)
{
    if (ImGui::IsItemClicked())
    {
        Engine::GetInstance().scene->SetSelected(gameObject);
        
        if (auto* inspector = Engine::GetInstance().editor->GetPanel<InspectorPanel>())
        {
            inspector->ClearSelectedResource();
        }
        
        if (auto* projectPanel = Engine::GetInstance().editor->GetPanel<ProjectPanel>())
        {
            projectPanel->ClearSelection();
        }
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        StartEditing(gameObject);
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("GameObjectContextMenu");
    }
}

void HierarchyTreeRenderer::RenderEditableNode(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene)
{
    strcpy(editBuffer, gameObject->GetName().c_str());
    ImGui::SetNextItemWidth(-1);
    ImGui::SetKeyboardFocusHere();
    
    if (ImGui::InputText("##edit", editBuffer, sizeof(editBuffer), 
                         ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
    {
        gameObject->SetName(editBuffer);
        editingObject = nullptr;
    }
    
    if (ImGui::IsItemDeactivated())
    {
        editingObject = nullptr;
    }
}

bool HierarchyTreeRenderer::IsEditing(std::shared_ptr<GameObject> gameObject) const
{
    return editingObject == gameObject;
}

void HierarchyTreeRenderer::StartEditing(std::shared_ptr<GameObject> gameObject)
{
    editingObject = gameObject;
}

void HierarchyTreeRenderer::StopEditing()
{
    editingObject = nullptr;
}

int HierarchyTreeRenderer::SetupNodeFlags(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected)
{
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

    return flags;
}

void HierarchyTreeRenderer::RenderDropZone(std::shared_ptr<GameObject> gameObject, HierarchyDragDrop* dragDropHandler)
{
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    float dropZoneHeight = 4.0f;
    
    ImGui::InvisibleButton("##drop_zone", ImVec2(regionAvail.x, dropZoneHeight));
    
    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        if (payload != nullptr && payload->IsDataType("GAMEOBJECT_PAYLOAD"))
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 lineY = ImVec2(cursorPos.x, cursorPos.y + dropZoneHeight / 2);
            drawList->AddLine(lineY, ImVec2(lineY.x + regionAvail.x, lineY.y), IM_COL32(255, 255, 0, 255), 2.0f);
        }
        
        ImGui::EndDragDropTarget();
    }
    
    dragDropHandler->HandleDropBetweenNodes(gameObject, nullptr);
}

bool HierarchyTreeRenderer::IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor)
{
    if (!potentialDescendant || !ancestor) return false;
    auto descendants = ancestor->GetAllDescendants();
    return std::find(descendants.begin(), descendants.end(), potentialDescendant) != descendants.end();
}
