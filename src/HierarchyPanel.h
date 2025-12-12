#pragma once

#include "ModuleEditor.h"
#include "imgui.h"
#include <memory>
#include <vector>

class GameObject;
class ModuleScene;

class HierarchyPanel : public EditorPanel
{
public:
    HierarchyPanel()
    {}

    // initial sizes and limits
    static constexpr float kDefaultFraction = 0.18f; // fracción del ancho de la ventana
    static constexpr float kMinPanelWidth = 80.0f;   // ancho mínimo del panel en px
    static constexpr float kMinCenterWidth = 180.0f; // ancho mínimo reservado para zona central

    bool Start() override;
    void SetupWindow(ImGuiViewport *viewport, float &panelWidth);
    void RenderContent(ModuleScene *scene, std::shared_ptr<GameObject> selected);
    void OnImGuiRender() override;
    void CleanUp() override;

private:
    struct PendingParentChange
    {
        std::shared_ptr<GameObject> gameObject;
        std::shared_ptr<GameObject> newParent;
    };

    // Tree rendering
    void RenderGameObjectTree(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene);
    void RenderTreeNode(std::shared_ptr<GameObject> gameObject, std::shared_ptr<GameObject> selected, ModuleScene* scene);
    void RenderHierarchyTree(ModuleScene* scene, std::shared_ptr<GameObject> selected);

    // Interactions and menus
    void HandleNodeInteraction(std::shared_ptr<GameObject> gameObject, ModuleScene* scene);
    void RenderGameObjectContextMenu(std::shared_ptr<GameObject> gameObject, ModuleScene* scene);
    void HandleHierarchyContextMenu(ModuleScene* scene);

    void HandleDragDrop(ModuleScene *scene);

    // Utilities
    void ProcessPendingParentChanges();
    static bool IsDescendant(std::shared_ptr<GameObject> potentialDescendant, std::shared_ptr<GameObject> ancestor);
    static void DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent, ModuleScene* scene);

    std::vector<PendingParentChange> pendingParentChanges;
    std::shared_ptr<GameObject> editingObject = nullptr;
    char editBuffer[256] = {0};
};
