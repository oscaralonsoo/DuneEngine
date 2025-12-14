#pragma once

#include "ModuleEditor.h"
#include "Component.h"
#include "Resource.h"
#include <memory>
#include <filesystem>

class InspectorPanel : public EditorPanel
{
public:
    InspectorPanel() = default;

    bool Start() override;

    void OnImGuiRender() override;     // Render the panel UI

    void CleanUp() override; //Clean up resources

    // Set the currently selected resource
    void SetSelectedResource(std::shared_ptr<Resource> resource, const std::filesystem::path& assetPath);

    // Clear the selected resource
    void ClearSelectedResource();

private:
    // Render a single component (Transform, Mesh, Material, etc.)
    void RenderComponent(Component* component, float panelWidth);

    // Render details of the selected resource
    void RenderResourceDetails();

    // Currently selected resource
    std::shared_ptr<Resource> mSelectedResource;

    // Path to the selected resource
    std::filesystem::path mSelectedResourcePath;
};
