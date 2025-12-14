#pragma once

#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "Resource.h"
#include "Texture.h"
#include "AssetSearch.h"
#include "imgui.h"
#include <filesystem>
#include <set>
#include <chrono>
#include <map>
#include <memory>
#include <glad/glad.h>
enum class ContextTarget
{
    None,
    Item,
    Background
};
class ProjectPanel : public EditorPanel
{
public:
    ProjectPanel()
    {}

    // initial sizes and limits
    static constexpr float kDefaultFraction = 0.18f; // fracción del ancho de la ventana
    static constexpr float kMinPanelWidth = 80.0f;   // ancho mínimo del panel en px
    static constexpr float kMinCenterWidth = 180.0f; // ancho mínimo reservado para zona central

    bool Start() override;
    void SetupWindow(ImGuiViewport *viewport, float &panelHeight);
    void RenderContent(ModuleScene *scene);
    void OnImGuiRender() override;
    void CleanUp() override;

    // Selection management
    void ClearSelection();

private:
    // Content view rendering
    void RenderContentView();
    void RenderAssetItem(const std::filesystem::path &assetPath);

    // Directory tree
    void RenderDirectoryTree(const std::filesystem::path& path, const std::filesystem::path& basePath);
    bool HasSubdirectories(const std::filesystem::path& path);

    // Asset operations
    void RefreshAssets();

    // Utilities
    std::string GetFileIcon(const std::filesystem::path& path);
    GLuint GetIconOrThumbnail(const std::filesystem::path& path);
    void RenderDeleteConfirmationModal();
    void RenderModalDialogs();
    void HandleExternalFileDrop(const std::filesystem::path& filePath);

    std::filesystem::path selectedFolder = "Assets";
    std::set<std::filesystem::path> selectedItems;
    std::chrono::steady_clock::time_point lastRefreshTime;

    // Pending operations for modal dialogs
    std::filesystem::path pendingRenamePath;
    std::set<std::filesystem::path> pendingDeletePaths;
    bool showDeleteModal = false;

    // In-place editing
    bool isEditing = false;
    bool justStartedEditing = false;
    std::filesystem::path editingPath;
    char editBuffer[256] = {0};
    std::map<std::filesystem::path, std::string> renamedThisFrame;

    // Thumbnails
    std::map<std::filesystem::path, std::shared_ptr<Texture>> thumbnailTextures;
    // Icon textures for different resource types
    std::map<ResourceType, std::shared_ptr<Texture>> iconTextures;
    // Dedicated folder icon texture
    std::shared_ptr<Texture> folderIconTexture;
    static constexpr float kThumbnailSize = 64.0f;
    static constexpr float kItemSpacing = 8.0f;

    // Search functionality
    AssetSearch assetSearch;
    char searchBuffer[256] = {0};

    
    ContextTarget activeContextTarget = ContextTarget::None;
    std::filesystem::path contextPath;
};
