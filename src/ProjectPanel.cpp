#include "ProjectPanel.h"
#include "Engine.h"
#include "ResourceUtils.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <chrono>

bool ProjectPanel::Start()
{
    return true;
}

void ProjectPanel::OnImGuiRender()
{
    RefreshAssets();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredHeight = workSize.y * ProjectPanel::kDefaultFraction;
    float otherPanelEstimate = workSize.y * ProjectPanel::kDefaultFraction;
    float maxAllowed = workSize.y - otherPanelEstimate - ProjectPanel::kMinCenterWidth;
    float panelHeight = std::clamp(desiredHeight, ProjectPanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(ImVec2(workPos.x, workPos.y + workSize.y - panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(workSize.x, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Project", nullptr, flags);

    // Split view: left for tree, right for content
    ImGui::Columns(2, "ProjectColumns", true);

    // Left column: Directory tree
    ImGui::BeginChild("TreeView", ImVec2(0, 0), true);
    std::filesystem::path assetsPath = "Assets";
    if (std::filesystem::exists(assetsPath))
    {
        RenderDirectoryTree(assetsPath, assetsPath);
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right column: Content view
    RenderContentView();

    ImGui::Columns(1);

    ImGui::End();

    // Render modal dialogs at the top level
    RenderModalDialogs();
}

void ProjectPanel::RenderDirectoryTree(const std::filesystem::path& path, const std::filesystem::path& basePath)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const auto& entryPath = entry.path();
        std::string filename = entryPath.filename().string();

        if (entry.is_directory())
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (entryPath == selectedFolder)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            std::string label = "[F] " + filename;
            bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

            if (ImGui::IsItemClicked())
            {
                selectedFolder = entryPath;
                selectedItems.clear();
            }

            HandleContextMenu(entryPath, true);

            if (nodeOpen)
            {
                RenderDirectoryTree(entryPath, basePath);
                ImGui::TreePop();
            }
        }
    }
}

void ProjectPanel::RenderContentView()
{
    ImGui::BeginChild("ContentView", ImVec2(0, 0), true);

    if (std::filesystem::exists(selectedFolder) && std::filesystem::is_directory(selectedFolder))
    {
        for (const auto& entry : std::filesystem::directory_iterator(selectedFolder))
        {
            const auto& entryPath = entry.path();
            std::string filename = entryPath.filename().string();
            std::string icon = GetFileIcon(entryPath);
            std::string label = icon + " " + filename;

            bool isSelected = selectedItems.find(entryPath) != selectedItems.end();

            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                if (ImGui::GetIO().KeyCtrl)
                {
                    if (isSelected)
                        selectedItems.erase(entryPath);
                    else
                        selectedItems.insert(entryPath);
                }
                else
                {
                    selectedItems.clear();
                    selectedItems.insert(entryPath);
                }
            }

            // Double-click folders to navigate
            if (entry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                selectedFolder = entryPath;
                selectedItems.clear();
            }

            HandleContextMenu(entryPath, entry.is_directory());

            if (!entry.is_directory() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                std::string relativePath = std::filesystem::relative(entryPath, "Assets").string();
                ImGui::SetDragDropPayload("ASSET_PAYLOAD", relativePath.c_str(), relativePath.size() + 1);
                ImGui::Text("Dragging %s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

    ImGui::EndChild();
}

void ProjectPanel::RefreshAssets()
{
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRefreshTime).count() < 2)
        return;

    lastRefreshTime = now;

    // Simple refresh check - in a real implementation, you'd track file modification times
    // For now, just ensure the selected folder still exists
    if (!std::filesystem::exists(selectedFolder))
    {
        selectedFolder = "Assets";
        selectedItems.clear();
    }
}

std::string ProjectPanel::GetFileIcon(const std::filesystem::path& path)
{
    if (std::filesystem::is_directory(path))
        return "[F]";

    ResourceType type = ResourceUtils::GetTypeFromExtension(path);
    switch (type)
    {
    case ResourceType::Texture: return "[T]";
    case ResourceType::Model: return "[M]";
    case ResourceType::Shader: return "[S]";
    case ResourceType::Material: return "[A]";
    default: return "[?]";
    }
}

void ProjectPanel::HandleContextMenu(const std::filesystem::path& path, bool isDirectory)
{
    std::string popupId = "AssetContextMenu_" + path.string();

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    if (ImGui::BeginPopup(popupId.c_str()))
    {
        if (ImGui::MenuItem("Rename"))
        {
            pendingRenamePath = path;
            ImGui::OpenPopup("Rename Asset");
        }
        if (ImGui::MenuItem("Delete"))
        {
            pendingDeletePath = path;
            ImGui::OpenPopup("Confirm Delete");
        }
        ImGui::EndPopup();
    }
}

void ProjectPanel::RenderModalDialogs()
{
    // Rename dialog
    if (!pendingRenamePath.empty())
    {
        static char renameBuffer[256] = {0};
        if (ImGui::BeginPopupModal("Rename Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            strcpy(renameBuffer, pendingRenamePath.filename().string().c_str());

            ImGui::Text("Enter new name:");
            ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer));

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                std::string newName = renameBuffer;
                if (!newName.empty())
                {
                    std::filesystem::path newPath = pendingRenamePath.parent_path() / newName;
                    try
                    {
                        std::filesystem::rename(pendingRenamePath, newPath);
                        // Update selection if renamed item was selected
                        if (selectedItems.find(pendingRenamePath) != selectedItems.end())
                        {
                            selectedItems.erase(pendingRenamePath);
                            selectedItems.insert(newPath);
                        }
                        if (selectedFolder == pendingRenamePath)
                        {
                            selectedFolder = newPath;
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        // TODO: Show error message
                    }
                }
                pendingRenamePath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                pendingRenamePath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // Delete confirmation
    if (!pendingDeletePath.empty())
    {
        if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to delete:");
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", pendingDeletePath.filename().string().c_str());
            ImGui::Text("This action cannot be undone!");

            if (ImGui::Button("Delete", ImVec2(120, 0)))
            {
                try
                {
                    std::filesystem::remove_all(pendingDeletePath);
                    // Remove from selection if deleted
                    selectedItems.erase(pendingDeletePath);
                    if (selectedFolder == pendingDeletePath || !std::filesystem::exists(selectedFolder))
                    {
                        selectedFolder = "Assets";
                    }
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    // TODO: Show error message
                }
                pendingDeletePath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                pendingDeletePath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void ProjectPanel::CleanUp()
{
}
