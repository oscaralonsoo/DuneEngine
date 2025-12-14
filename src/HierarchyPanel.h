#pragma once

#include "ModuleEditor.h"
#include "HierarchyDragDrop.h"
#include "HierarchyTreeRenderer.h"
#include <imgui.h>
#include <memory>

class GameObject;
class ModuleScene;

class HierarchyPanel : public EditorPanel
{
public:
    HierarchyPanel();
    ~HierarchyPanel() = default;

    // Panel size configuration
    static constexpr float kDefaultFraction = 0.18f; 
    static constexpr float kMinPanelWidth = 80.0f;  
    static constexpr float kMinCenterWidth = 180.0f; 

    bool Start() override;
    void SetupWindow(ImGuiViewport* viewport, float& panelWidth);
    void RenderContent(ModuleScene* scene, std::shared_ptr<GameObject> selected);
    void OnImGuiRender() override;
    void CleanUp() override;

    // Context menu rendering (called by HierarchyNodeRenderer)
    void RenderNodeContextMenu(std::shared_ptr<GameObject> gameObject, ModuleScene* scene);
    void RenderBackgroundContextMenu(ModuleScene* scene);

private:
    // Render the hierarchy tree with all root GameObjects
    void RenderHierarchyTree(ModuleScene* scene, std::shared_ptr<GameObject> selected);
    
    // Handle background interactions
    void HandleBackgroundInteraction(ModuleScene* scene);
    
    // Render the create submenu (primitives, camera, etc.)
    void RenderCreateMenu(std::shared_ptr<GameObject> parent, ModuleScene* scene);

    // Specialized handlers for different responsibilities
    HierarchyDragDrop dragDropHandler;
    HierarchyTreeRenderer treeRenderer;
};
