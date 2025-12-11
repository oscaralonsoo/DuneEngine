#include "ProjectPanel.h"
#include "Engine.h"
#include "ResourceUtils.h"
#include "Texture.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <map>
#include <fstream>

bool ProjectPanel::Start()
{
    std::filesystem::path placeholderIcon = "icons/folder.png";
    if (std::filesystem::exists(placeholderIcon))
    {
        iconTextures[ResourceType::Unknown] = std::make_unique<Texture>(placeholderIcon, true, true);
    }

    return true;
}

// Sets up the ImGui window for the ProjectPanel
void ProjectPanel::SetupWindow(ImGuiViewport* viewport, float& panelHeight)
{
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredHeight = workSize.y * ProjectPanel::kDefaultFraction;
    float otherPanelEstimate = workSize.y * ProjectPanel::kDefaultFraction;
    float maxAllowed = workSize.y - otherPanelEstimate - ProjectPanel::kMinCenterWidth;
    panelHeight = std::clamp(desiredHeight, ProjectPanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(ImVec2(workPos.x, workPos.y + workSize.y - panelHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(workSize.x, panelHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(viewport->ID);
}

// Renders the main content of the ProjectPanel
void ProjectPanel::RenderContent(ModuleScene* scene)
{
    ImGui::Columns(2, "ProjectColumns", true);

    ImGui::BeginChild("TreeView", ImVec2(0, 0), true);
    std::filesystem::path assetsPath = "Assets";
    if (std::filesystem::exists(assetsPath))
    {
        RenderDirectoryTree(assetsPath, assetsPath);
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    RenderContentView();

    ImGui::Columns(1);
}

void ProjectPanel::OnImGuiRender()
{
    RefreshAssets();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float panelHeight;

    SetupWindow(viewport, panelHeight);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Project", nullptr, flags);

    RenderContent(nullptr); // scene not needed here

    ImGui::End();

    RenderModalDialogs();

    renamedThisFrame.clear();
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

            std::string label = filename;
            std::string uniqueId = entryPath.string();

            GLuint textureID = GetIconOrThumbnail(entryPath);
            if (textureID != 0)
            {
                ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(16, 16));
                ImGui::SameLine();
            }

            bool nodeOpen = ImGui::TreeNodeEx((label + "##" + uniqueId).c_str(), flags);

            if (ImGui::IsItemClicked())
            {
                selectedFolder = entryPath;
                selectedItems.clear();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
                {
                    std::string pathStr = (const char*)payload->Data;
                    std::filesystem::path draggedPath = pathStr;
                    std::filesystem::path newPath = entryPath / draggedPath.filename();
                    try
                    {
                        std::filesystem::rename(draggedPath, newPath);
                        lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                    }
                    catch (const std::filesystem::filesystem_error&)
                    {
                    }
                }
                ImGui::EndDragDropTarget();
            }

            bool dummy = false;
            HandleContextMenu(entryPath, true, dummy);

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

    bool rightClickedOnItem = false;

    // Search bar
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##Search", "Search assets...", searchBuffer, sizeof(searchBuffer)))
    {
        assetSearch.SetQuery(searchBuffer);
        // Clear selection when search changes
        selectedItems.clear();
    }
    ImGui::PopItemWidth();

    // Clear search button
    if (assetSearch.IsActive())
    {
        ImGui::SameLine();
        if (ImGui::Button("X"))
        {
            searchBuffer[0] = '\0';
            assetSearch.Clear();
            selectedItems.clear();
        }
    }

    ImGui::Separator();

    // Back button
    if (selectedFolder.has_parent_path() && selectedFolder.parent_path() != selectedFolder)
    {
        if (ImGui::Button("Back"))
        {
            selectedFolder = selectedFolder.parent_path();
            selectedItems.clear();
        }
        ImGui::Separator();
    }

    if (std::filesystem::exists(selectedFolder) && std::filesystem::is_directory(selectedFolder))
    {
        float availWidth = ImGui::GetContentRegionAvail().x;
        float itemWidth = kThumbnailSize + 8.0f;
        int itemsPerRow = std::max(2, static_cast<int>(availWidth / itemWidth));

        ImGui::Columns(itemsPerRow, "AssetGrid", false);

        //recursive iterator when searching & regular iterator when not searching
        if (assetSearch.IsActive())
        {
            //search through all subfolders
            for (const auto& entry : std::filesystem::recursive_directory_iterator(selectedFolder))
            {
                const auto& entryPath = entry.path();
                if (assetSearch.Matches(entryPath))
                {
                    RenderAssetItem(entryPath, rightClickedOnItem);
                }
            }
        }
        else
        {
            // Regular display for normal browsing
            for (const auto& entry : std::filesystem::directory_iterator(selectedFolder))
            {
                const auto& entryPath = entry.path();
                RenderAssetItem(entryPath, rightClickedOnItem);
            }
        }

        ImGui::Columns(1);
    }

    ImGui::EndChild();
}

void ProjectPanel::RefreshAssets()
{
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRefreshTime).count() < 2)
        return;

    lastRefreshTime = now;

    if (!std::filesystem::exists(selectedFolder))
    {
        selectedFolder = "Assets";
        selectedItems.clear();
    }
}

std::string ProjectPanel::GetFileIcon(const std::filesystem::path& path)
{
    ResourceType type = ResourceUtils::GetTypeFromExtension(path);
    switch (type)
    {
    case ResourceType::Texture: return "[TEX]"; 
    case ResourceType::Model: return "[MOD]"; 
    case ResourceType::Shader: return "[SHD]"; 
    case ResourceType::Material: return "[MAT]"; 
    default: return "[?]"; 
    }
}

GLuint ProjectPanel::GetIconOrThumbnail(const std::filesystem::path& path)
{
    if (std::filesystem::is_directory(path))
    {
        auto it = iconTextures.find(ResourceType::Unknown);
        return (it != iconTextures.end()) ? it->second->GetID() : 0;
    }

    auto thumbIt = thumbnailTextures.find(path);
    if (thumbIt != thumbnailTextures.end())
        return thumbIt->second->GetID();

    ResourceType type = ResourceUtils::GetTypeFromExtension(path);
    if (type == ResourceType::Texture)
    {
        thumbnailTextures[path] = std::make_unique<Texture>(path, true);
        return thumbnailTextures[path]->GetID();
    }
    else
    {
        auto iconIt = iconTextures.find(type);
        return (iconIt != iconTextures.end()) ? iconIt->second->GetID() : 0;
    }
}

void ProjectPanel::HandleContextMenu(const std::filesystem::path& path, bool isDirectory, bool& rightClickedOnItem)
{
    std::string popupId = "AssetContextMenu_" + path.string();

    if (ImGui::IsItemClicked(1))
    {
        ImGui::OpenPopup(popupId.c_str());
        rightClickedOnItem = true;
    }

    if (ImGui::BeginPopup(popupId.c_str()))
    {
        if (ImGui::MenuItem("Rename"))
        {
            isEditing = true;
            editingPath = path;
             strcpy_s(editBuffer, sizeof(editBuffer), path.filename().string().c_str());
        }
        if (ImGui::MenuItem("Delete"))
        {
            pendingDeletePaths = {path};
            showDeleteModal = true;
        }
        ImGui::Separator();

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Folder"))
            {
                std::filesystem::path newFolderPath = selectedFolder / "New Folder";
                int counter = 1;
                while (std::filesystem::exists(newFolderPath))
                {
                    newFolderPath = selectedFolder / ("New Folder " + std::to_string(counter));
                    counter++;
                }
                try
                {
                    std::filesystem::create_directory(newFolderPath);
                    isEditing = true;
                    editingPath = newFolderPath;
                    strcpy_s(editBuffer, sizeof(editBuffer), newFolderPath.filename().string().c_str());
                }
                catch (const std::filesystem::filesystem_error&)
                {
                }
            }
            if (ImGui::MenuItem("GameObject"))
            {
                std::filesystem::path newPrefabPath = selectedFolder / "New GameObject.prefab";
                int counter = 1;
                while (std::filesystem::exists(newPrefabPath))
                {
                    newPrefabPath = selectedFolder / ("New GameObject " + std::to_string(counter) + ".prefab");
                    counter++;
                }
                try
                {
                    // Create an empty prefab file
                    std::ofstream file(newPrefabPath);
                    file.close();
                    isEditing = true;
                    editingPath = newPrefabPath;
                    strcpy_s(editBuffer, sizeof(editBuffer), newPrefabPath.filename().string().c_str());
                }
                catch (const std::filesystem::filesystem_error&)
                {
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

// Renders the delete confirmation modal dialog
void ProjectPanel::RenderDeleteConfirmationModal()
{
    if (!pendingDeletePaths.empty())
    {
        if (ImGui::BeginPopup("DeleteConfirmation", ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (pendingDeletePaths.size() == 1)
            {
                ImGui::Text("Are you sure you want to delete:");
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", (*pendingDeletePaths.begin()).filename().string().c_str());
            }
            else
            {
                ImGui::Text("Are you sure you want to delete %d items?", (int)pendingDeletePaths.size());
                ImGui::Text("This includes:");
                for (const auto& path : pendingDeletePaths)
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "  %s", path.filename().string().c_str());
                }
            }
            ImGui::Text("This action cannot be undone!");

            if (ImGui::Button("Delete", ImVec2(120, 0)))
            {
                try
                {
                    for (const auto& path : pendingDeletePaths)
                    {
                        std::filesystem::remove_all(path);
                        // Remove from selection if deleted
                        selectedItems.erase(path);
                        if (selectedFolder == path || !std::filesystem::exists(selectedFolder))
                        {
                            selectedFolder = "Assets";
                        }
                    }
                    lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                }
                catch (const std::filesystem::filesystem_error&)
                {
                }
                pendingDeletePaths.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                pendingDeletePaths.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void ProjectPanel::RenderModalDialogs()
{
    if (showDeleteModal)
    {
        ImGui::OpenPopup("DeleteConfirmation");
        showDeleteModal = false;
    }

    RenderDeleteConfirmationModal();
}

void ProjectPanel::RenderAssetItem(const std::filesystem::path& assetPath, bool& rightClickedOnItem)
{
    std::string filename = assetPath.filename().string();
    bool isSelected = selectedItems.find(assetPath) != selectedItems.end();

    ImVec2 itemPos = ImGui::GetCursorPos();

    GLuint textureID = GetIconOrThumbnail(assetPath);
    if (textureID != 0)
    {
        ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(kThumbnailSize, kThumbnailSize));
    }
    else
    {
        std::string icon = GetFileIcon(assetPath);
        ImGui::Button(icon.c_str(), ImVec2(kThumbnailSize, kThumbnailSize));
    }

    ImVec2 textPos = ImGui::GetCursorPos();

    std::string displayName = filename;
    if (displayName.length() > 12)
    {
        displayName = displayName.substr(0, 9) + "...";
    }

    if (isEditing && editingPath == assetPath)
    {
        ImGui::SetCursorPos(textPos);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 5));
        ImGui::PushItemWidth(-1);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##edit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            std::string newName = editBuffer;
            if (!newName.empty() && newName != filename)
            {
                std::filesystem::path newPath = assetPath.parent_path() / newName;
                try
                {
                    std::filesystem::rename(assetPath, newPath);
                    renamedThisFrame[assetPath] = newName;
                    selectedItems.erase(assetPath);
                    selectedItems.insert(newPath);
                    lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                }
                catch (const std::filesystem::filesystem_error&)
                {
                }
            }
            isEditing = false;
        }
        if (ImGui::IsItemDeactivated())
        {
            isEditing = false;
        }
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            isEditing = false;
        }
        HandleContextMenu(assetPath, std::filesystem::is_directory(assetPath), rightClickedOnItem);
    }
    else
    {
        ImGui::Text(displayName.c_str());

        ImGui::SetCursorPos(itemPos);
        if (ImGui::Selectable(("##" + assetPath.string()).c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(kThumbnailSize, kThumbnailSize + 20.0f)))
        {
            if (ImGui::GetIO().KeyCtrl)
            {
                if (isSelected)
                    selectedItems.erase(assetPath);
                else
                    selectedItems.insert(assetPath);
            }
            else
            {
                selectedItems.clear();
                selectedItems.insert(assetPath);
            }

            if (ImGui::IsMouseDoubleClicked(0))
            {
                if (std::filesystem::is_directory(assetPath))
                {
                    selectedFolder = assetPath;
                    selectedItems.clear();
                }
            }
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            std::string relativePath = std::filesystem::relative(assetPath, "Assets").string();
            ImGui::SetDragDropPayload("ASSET_PAYLOAD", relativePath.c_str(), relativePath.size() + 1);

            ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
            ImGui::Text("Dragging: %s (%s)", filename.c_str(), ResourceUtils::ToString(type).c_str());

            ImGui::EndDragDropSource();
        }

        HandleContextMenu(assetPath, std::filesystem::is_directory(assetPath), rightClickedOnItem);
    }

    ImGui::NextColumn();
}
void ProjectPanel::CleanUp()
{
    thumbnailTextures.clear();
    iconTextures.clear();
}
