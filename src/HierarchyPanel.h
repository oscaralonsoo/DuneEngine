#pragma once

#include "ModuleEditor.h"
#include "imgui.h"
#include <memory>

class GameObject;

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
    void OnImGuiRender() override;
    void CleanUp() override;

private:
    void RenderGameObjectTree(std::shared_ptr<GameObject> go, std::shared_ptr<GameObject> selected);
};
