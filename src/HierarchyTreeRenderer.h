#pragma once

#include <memory>

class GameObject;
class ModuleScene;
class HierarchyDragDrop;
class HierarchyPanel;

class HierarchyTreeRenderer
{
public:
    HierarchyTreeRenderer();
    ~HierarchyTreeRenderer() = default;

    // Render a single GameObject node with all features
    void RenderNode(
        std::shared_ptr<GameObject> gameObject,
        std::shared_ptr<GameObject> selected,
        ModuleScene* scene,
        HierarchyDragDrop* dragDropHandler,
        HierarchyPanel* hierarchyPanel
    );
    
    // Name editing functions
    bool IsEditing(std::shared_ptr<GameObject> gameObject) const;
    void StartEditing(std::shared_ptr<GameObject> gameObject);
    void StopEditing();

private:
    // Setup ImGui tree node flags based on GameObject state
    int SetupNodeFlags(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected);
    
    // Handle node interactions (click, double-click, right-click)
    void HandleNodeInteraction(std::shared_ptr<GameObject> gameObject, ModuleScene* scene);
    
    // Render drop zone between nodes for reordering
    void RenderDropZone(std::shared_ptr<GameObject> gameObject, HierarchyDragDrop* dragDropHandler);
    
    // Render editable name input for a GameObject
    void RenderEditableNode(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene);
    
    // Helper function to check if a GameObject is a descendant of another
    bool IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor);
    
    // Name editing state
    std::shared_ptr<GameObject> editingObject;
    char editBuffer[256];
};
