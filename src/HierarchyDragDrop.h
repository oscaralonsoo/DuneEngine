#pragma once

#include <memory>
#include <vector>

class GameObject;
class ModuleScene;

// Handles drag & drop logic in the hierarchy panel
class HierarchyDragDrop
{
public:
    // Struct representing a pending parent change for a GameObject
    struct PendingParentChange
    {
        std::shared_ptr<GameObject> gameObject;
        std::shared_ptr<GameObject> newParent;
        int insertIndex = -1; // -1 means append at end
    };

    HierarchyDragDrop();
    ~HierarchyDragDrop() = default;

    // Handle drag source for a GameObject
    void HandleDragSource(std::shared_ptr<GameObject> gameObject);

    // Handle drop target for a GameObject node
    void HandleNodeDropTarget(std::shared_ptr<GameObject> targetGameObject, ModuleScene* scene);
    
    // Handle drop between nodes
    void HandleDropBetweenNodes(std::shared_ptr<GameObject> afterGameObject, ModuleScene* scene);

    // Handle drop target for the hierarchy background
    void HandleBackgroundDropTarget(ModuleScene* scene);

    // Process all pending parent changes
    void ProcessPendingChanges();

    // Check if there are pending changes
    bool HasPendingChanges() const { return !pendingParentChanges.empty(); }

    // Add a pending parent change
    void AddPendingChange(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> newParent, int insertIndex = -1)
    {
        pendingParentChanges.push_back({gameObject, newParent, insertIndex});
    }

private:
    // Check if potentialDescendant is a descendant of ancestor
    static bool IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor);

    // Handle GameObject payload drop
    void HandleGameObjectDrop(std::shared_ptr<GameObject> dragged, std::shared_ptr<GameObject> target);

    // Handle asset file payload drop
    void HandleAssetDrop(const char* relativePath, std::shared_ptr<GameObject> parent, ModuleScene* scene);

    // Handle external file payload drop
    void HandleFileDrop(const char* files, std::shared_ptr<GameObject> parent, ModuleScene* scene);

    // Pending parent changes to apply after drag & drop
    std::vector<PendingParentChange> pendingParentChanges;
};
