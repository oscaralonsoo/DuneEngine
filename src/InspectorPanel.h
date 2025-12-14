#pragma once

#include "ModuleEditor.h"
#include "Component.h"
#include "Resource.h"
#include <memory>
#include <filesystem>

class InspectorPanel : public EditorPanel
{
    public:
        InspectorPanel()
        {}

        bool Start() override;
        void OnImGuiRender() override;
        void CleanUp() override;

        // Resource selection
        void SetSelectedResource(std::shared_ptr<Resource> resource, const std::filesystem::path& assetPath);
        void ClearSelectedResource();

    private:
        void RenderComponent(Component* component, float panelWidth);
        void RenderResourceDetails();

        // Selected resource
        std::shared_ptr<Resource> mSelectedResource;
        std::filesystem::path mSelectedResourcePath;
};
