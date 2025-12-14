#pragma once

#include "Resource.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include <memory>
#include <string>
#include <filesystem>

namespace ResourceUI
{
    // Helper functions
    std::string FormatMemorySize(size_t bytes);
    std::string GetResourceTypeName(ResourceType type);

    // Main rendering function
    void RenderResourceDetails(
        std::shared_ptr<Resource> resource,
        const std::filesystem::path& assetPath
    );

    // Resource info rendering
    void RenderResourceHeader(const std::string& filename);
    void RenderResourceInfo(
        std::shared_ptr<Resource> resource,
        const std::filesystem::path& assetPath,
        ResourceType type
    );

    // Type-specific detail renderers
    void RenderTextureDetails(std::shared_ptr<Texture> texture);
    void RenderMeshDetails(std::shared_ptr<Mesh> mesh);
    void RenderMaterialDetails(std::shared_ptr<Material> material);
    void RenderModelDetails(std::shared_ptr<Model> model);

    // Usage info rendering
    void RenderUsageInfo(std::shared_ptr<Resource> resource);
}
