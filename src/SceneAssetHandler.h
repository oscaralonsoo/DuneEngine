#pragma once

#include <filesystem>
#include <imgui.h>
#include "Framebuffer.h"

class SceneAssetHandler
{
public:
    SceneAssetHandler() = default;
    ~SceneAssetHandler() = default;

    void HandleDragDrop(ImVec2 viewSize, float mouseX, float mouseY, Framebuffer* framebuffer);
    void HandleAssetDrop(const std::filesystem::path& assetPath, float mouseX, float mouseY, 
                        uint32_t width, uint32_t height);

private:
    void ApplyTextureToObject(class GameObject* object, const std::filesystem::path& texturePath);
    void InstantiateModel(const std::filesystem::path& modelPath);
    void InstantiatePrefab(const std::filesystem::path& prefabPath);
};
