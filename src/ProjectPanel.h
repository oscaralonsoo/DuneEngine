#pragma once

#include "ModuleEditor.h"
#include "imgui.h"
#include <filesystem>
#include <set>
#include <chrono>

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
    void OnImGuiRender() override;
    void CleanUp() override;

private:
    void RenderDirectoryTree(const std::filesystem::path& path, const std::filesystem::path& basePath);
    void RenderContentView();
    void RefreshAssets();
    std::string GetFileIcon(const std::filesystem::path& path);
    void HandleContextMenu(const std::filesystem::path& path, bool isDirectory);
    void RenderModalDialogs();

    std::filesystem::path selectedFolder = "Assets";
    std::set<std::filesystem::path> selectedItems;
    std::chrono::steady_clock::time_point lastRefreshTime;
    std::filesystem::file_time_type lastAssetsWriteTime;

    // Pending operations for modal dialogs
    std::filesystem::path pendingRenamePath;
    std::filesystem::path pendingDeletePath;
};
