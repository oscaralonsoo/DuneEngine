#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include "GameObject.h"
#include <imgui.h>

HierarchyPanel::HierarchyPanel()
    : dragDropHandler()
    , treeRenderer()
{
}

bool HierarchyPanel::Start()
{
    return true;
}

void HierarchyPanel::SetupWindow(ImGuiViewport* viewport, float& panelWidth)
{
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredWidth = workSize.x * kDefaultFraction;
    float otherPanelEstimate = workSize.x * kDefaultFraction;
    float maxAllowed = workSize.x - otherPanelEstimate - kMinCenterWidth;
    panelWidth = std::clamp(desiredWidth, kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(workPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(viewport->ID);
}

void HierarchyPanel::RenderContent(ModuleScene* scene, std::shared_ptr<GameObject> selected)
{
    RenderHierarchyTree(scene, selected);
    RenderBackgroundContextMenu(scene);
}

void HierarchyPanel::OnImGuiRender()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Hierarchy", nullptr, flags);

    if (auto scene = Engine::GetInstance().scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        RenderHierarchyTree(scene.get(), selected);
        RenderBackgroundContextMenu(scene.get());
    }

    ImGui::End();
}

void HierarchyPanel::RenderHierarchyTree(ModuleScene* scene, std::shared_ptr<GameObject> selected)
{
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // Render all root GameObjects
    for (std::shared_ptr<GameObject> gameObject : scene->GetGameObjects())
    {
        if (!gameObject->GetParent())
        {
            treeRenderer.RenderNode(gameObject, selected, scene, &dragDropHandler, this);
        }
    }

    // Invisible button for background interactions
    ImGui::SetCursorPos(ImVec2(0, ImGui::GetCursorPosY()));
    ImGui::InvisibleButton("HierarchyBackground", ImVec2(contentSize.x, ImGui::GetContentRegionAvail().y));

    HandleBackgroundInteraction(scene);

    dragDropHandler.HandleBackgroundDropTarget(scene);
    
    if (dragDropHandler.HasPendingChanges())
    {
        dragDropHandler.ProcessPendingChanges();
    }
}

void HierarchyPanel::HandleBackgroundInteraction(ModuleScene* scene)
{
    if (ImGui::IsItemClicked() && scene->GetSelected())
    {
        scene->SetSelected(nullptr);
        
        if (auto* inspector = Engine::GetInstance().editor->GetPanel<InspectorPanel>())
        {
            inspector->ClearSelectedResource();
        }
        
        if (auto* projectPanel = Engine::GetInstance().editor->GetPanel<ProjectPanel>())
        {
            projectPanel->ClearSelection();
        }
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("HierarchyContextMenu");
    }
}

void HierarchyPanel::RenderNodeContextMenu(std::shared_ptr<GameObject> gameObject, ModuleScene* scene)
{
    if (ImGui::BeginPopup("GameObjectContextMenu"))
    {
        if (ImGui::MenuItem("Rename"))
        {
            treeRenderer.StartEditing(gameObject);
        }
        
        if (ImGui::MenuItem("Duplicate"))
        {
            treeRenderer.StopEditing();
            scene->DuplicateGameObject(gameObject, gameObject->GetParent());
        }
        
        if (ImGui::BeginMenu("Create Child"))
        {
            RenderCreateMenu(gameObject, scene);
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem("Unparent"))
        {
            treeRenderer.StopEditing();
            gameObject->SetParent(nullptr);
        }
        
        if (ImGui::MenuItem("Delete"))
        {
            treeRenderer.StopEditing();
            Engine::GetInstance().scene->RemoveGameObject(gameObject);
        }
        
        ImGui::EndPopup();
    }
}

void HierarchyPanel::RenderBackgroundContextMenu(ModuleScene* scene)
{
    if (ImGui::BeginPopup("HierarchyContextMenu"))
    {
        RenderCreateMenu(nullptr, scene);
        ImGui::EndPopup();
    }
}

void HierarchyPanel::RenderCreateMenu(std::shared_ptr<GameObject> parent, ModuleScene* scene)
{
    if (ImGui::MenuItem("Empty GameObject"))
    {
        auto child = scene->CreateEmptyGameObject();
        if (parent) child->SetParent(parent);
    }
    
    ImGui::Separator();
    
    if (ImGui::BeginMenu("3D Object"))
    {
        if (ImGui::MenuItem("Cube"))
        {
            auto child = scene->CreateCube();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Sphere"))
        {
            auto child = scene->CreateSphere();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Capsule"))
        {
            auto child = scene->CreateCapsule();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Cylinder"))
        {
            auto child = scene->CreateCylinder();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Cone"))
        {
            auto child = scene->CreateCone();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Plane"))
        {
            auto child = scene->CreatePlane();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Quad"))
        {
            auto child = scene->CreateQuad();
            if (parent) child->SetParent(parent);
        }
        if (ImGui::MenuItem("Torus"))
        {
            auto child = scene->CreateTorus();
            if (parent) child->SetParent(parent);
        }
        ImGui::EndMenu();
    }
    
    if (ImGui::MenuItem("Camera"))
    {
        auto child = scene->CreateCamera();
        if (parent) child->SetParent(parent);
    }
}

void HierarchyPanel::CleanUp()
{
}
