#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <imgui.h>
#include "Framebuffer.h"

class GameObject;

class SceneAssetHandler
{
public:
    SceneAssetHandler() = default;
    ~SceneAssetHandler() = default;

    void HandleDragDrop(ImVec2 viewSize, float mouseX, float mouseY, Framebuffer* framebuffer);
    void HandleAssetDrop(const std::filesystem::path& assetPath, float mouseX, float mouseY, 
                        uint32_t width, uint32_t height);

    // Hover state management for drag preview
    GameObject* GetHoveredObject() const { return m_HoveredObject; }
    bool IsTextureDragging() const { return m_IsTextureDragging; }
    void ClearHoverState();

private:
    void ApplyTextureToObject(GameObject* object, const std::filesystem::path& texturePath);
    void InstantiateModel(const std::filesystem::path& modelPath);
    void InstantiatePrefab(const std::filesystem::path& prefabPath);
    void UpdateHoverState(float mouseX, float mouseY, uint32_t width, uint32_t height, bool isDraggingTexture);

    // Hover state for drag preview
    GameObject* m_HoveredObject = nullptr;
    bool m_IsTextureDragging = false;
};
