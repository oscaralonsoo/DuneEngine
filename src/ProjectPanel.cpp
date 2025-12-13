#include "ProjectPanel.h"
#include "Engine.h"
#include "ResourceUtils.h"
#include "Texture.h"
#include "ModuleResource.h"

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
        folderIconTexture = std::dynamic_pointer_cast<Texture>(Engine::GetInstance().resourceManager->RequestResource(placeholderIcon));
    }

    // Load model icon
    std::filesystem::path modelIcon = "icons/model.png";
    if (std::filesystem::exists(modelIcon))
    {
        iconTextures[ResourceType::Model] = std::dynamic_pointer_cast<Texture>(Engine::GetInstance().resourceManager->RequestResource(modelIcon));
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

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Project", nullptr, flags);

    RenderContent(nullptr);

    ImGui::End();

    RenderModalDialogs();
    renamedThisFrame.clear();
}

bool ProjectPanel::HasSubdirectories(const std::filesystem::path& path)
{
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
                return true;
        }
    }
    catch (...)
    {
        // If we can't read the directory, assume it has no subdirectories
        return false;
    }
    return false;
}

void ProjectPanel::RenderDirectoryTree(const std::filesystem::path& path,
                                       const std::filesystem::path& basePath)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const auto& entryPath = entry.path();
        std::string filename = entryPath.filename().string();

        if (!entry.is_directory())
            continue;

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick;

        if (entryPath == selectedFolder)
            flags |= ImGuiTreeNodeFlags_Selected;

        // Si la carpeta no tiene subcarpetas, agregar flag Leaf para ocultar la flecha
        if (!HasSubdirectories(entryPath))
            flags |= ImGuiTreeNodeFlags_Leaf;

        GLuint textureID = GetIconOrThumbnail(entryPath);
        if (textureID != 0)
        {
            // Voltear la textura verticalmente usando coordenadas UV
            ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SameLine();
        }

        bool nodeOpen = ImGui::TreeNodeEx(
            (filename + "##" + entryPath.string()).c_str(), flags);

        // Click izquierdo → seleccionar carpeta
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            selectedFolder = entryPath;
            selectedItems.clear();
        }

        // Click derecho → menú contextual
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (selectedItems.find(entryPath) == selectedItems.end())
            {
                selectedItems.clear();
                selectedItems.insert(entryPath);
            }
        }

        if (ImGui::BeginPopupContextItem())
        {
            activeContextTarget = ContextTarget::Item;

            if (selectedItems.size() == 1)
                contextPath = *selectedItems.begin();
            else
                contextPath.clear();

            if (ImGui::MenuItem("Rename", nullptr, false, selectedItems.size() == 1))
            {
                isEditing = true;
                justStartedEditing = true;
                editingPath = *selectedItems.begin();
                std::string filename = editingPath.filename().string();
                strncpy_s(editBuffer, sizeof(editBuffer), filename.c_str(), sizeof(editBuffer) - 1);
                editBuffer[sizeof(editBuffer) - 1] = '\0';
            }

            if (ImGui::MenuItem("Delete"))
            {
                pendingDeletePaths = selectedItems;
                showDeleteModal = true;
            }

            ImGui::EndPopup();
        }

        // Drag & Drop Target
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
            {
                std::filesystem::path draggedPath = (const char*)payload->Data;
                std::filesystem::path newPath = entryPath / draggedPath.filename();
                try
                {
                    std::filesystem::rename(draggedPath, newPath);
                    lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                }
                catch (...) {}
            }
            ImGui::EndDragDropTarget();
        }

        if (nodeOpen)
        {
            RenderDirectoryTree(entryPath, basePath);
            ImGui::TreePop();
        }
    }
}

void ProjectPanel::RenderContentView()
{
    ImGui::BeginChild("ContentView", ImVec2(0, 0), true);

    // --- Search ---
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##Search", "Search assets...", searchBuffer, sizeof(searchBuffer)))
    {
        assetSearch.SetQuery(searchBuffer);
        selectedItems.clear();
    }
    ImGui::PopItemWidth();

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

    // --- Botón Back ---
    if (selectedFolder.has_parent_path() && selectedFolder.parent_path() != selectedFolder)
    {
        if (ImGui::Button("Back"))
        {
            selectedFolder = selectedFolder.parent_path();
            selectedItems.clear();
        }
        ImGui::Separator();
    }

    if (!std::filesystem::exists(selectedFolder))
    {
        ImGui::EndChild();
        return;
    }

    float availWidth = ImGui::GetContentRegionAvail().x;
    float itemWidth = kThumbnailSize + 8.0f;
    int itemsPerRow = std::max(2, (int)(availWidth / itemWidth));

    ImGui::Columns(itemsPerRow, "AssetGrid", false);

    std::filesystem::path hoveredItem;
    bool anyItemHovered = false;

    // --- Render de items ---
    for (const auto& entry : std::filesystem::directory_iterator(selectedFolder))
    {
        // Skip .meta files
        if (entry.path().extension() == ".meta")
            continue;

        RenderAssetItem(entry.path());

        if (ImGui::IsItemHovered())
        {
            hoveredItem = entry.path();
            anyItemHovered = true;
        }
    }

    ImGui::Columns(1);

    ImGuiIO& io = ImGui::GetIO();

    // --- Click away → deseleccionar ---
    if (ImGui::IsWindowHovered() && io.MouseClicked[0] && !anyItemHovered)
    {
        selectedItems.clear();
    }

    // --- Click derecho → seleccionar + abrir popup ---
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && io.MouseClicked[1])
    {
        if (anyItemHovered)
        {
            if (selectedItems.find(hoveredItem) == selectedItems.end())
            {
                selectedItems.clear();
                selectedItems.insert(hoveredItem);
            }
            activeContextTarget = ContextTarget::Item;
            contextPath = hoveredItem;
        }
        else
        {
            selectedItems.clear();
            activeContextTarget = ContextTarget::Background;
            contextPath.clear();
        }

        ImGui::OpenPopup("ProjectContextMenu");
    }

    // --- Popup contextual ---
    if (ImGui::BeginPopup("ProjectContextMenu"))
    {
        if (activeContextTarget == ContextTarget::Item && !selectedItems.empty())
        {
            contextPath = *selectedItems.begin();

            if (ImGui::MenuItem("Rename", nullptr, false, selectedItems.size() == 1))
            {
                isEditing = true;
                justStartedEditing = true;
                editingPath = contextPath;
                std::string filename = contextPath.filename().string();
                strncpy_s(editBuffer, sizeof(editBuffer), filename.c_str(), sizeof(editBuffer) - 1);
                editBuffer[sizeof(editBuffer) - 1] = '\0';
            }

            if (ImGui::MenuItem("Delete"))
            {
                pendingDeletePaths = selectedItems;
                showDeleteModal = true;
            }
        }
        else if (activeContextTarget == ContextTarget::Background)
        {
            if (ImGui::MenuItem("Create Folder"))
            {
                std::filesystem::path newFolder = selectedFolder / "New Folder";
                int i = 1;
                while (std::filesystem::exists(newFolder))
                    newFolder = selectedFolder / ("New Folder " + std::to_string(i++));
                std::filesystem::create_directory(newFolder);
                lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
            }
        }

        ImGui::EndPopup();
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
    // Return folder icon for directories
    if (std::filesystem::is_directory(path))
    {
        return folderIconTexture ? folderIconTexture->GetID() : 0;
    }

    // Check if we already have a thumbnail cached
    auto thumbIt = thumbnailTextures.find(path);
    if (thumbIt != thumbnailTextures.end())
        return thumbIt->second->GetID();

    // Get resource type for the file
    ResourceType type = ResourceUtils::GetTypeFromExtension(path);
    
    // For texture files, load and cache the actual texture as thumbnail
    if (type == ResourceType::Texture)
    {
        thumbnailTextures[path] = std::dynamic_pointer_cast<Texture>(Engine::GetInstance().resourceManager->RequestResource(path));
        return thumbnailTextures[path]->GetID();
    }
    
    // For other file types, try to find a specific icon
    // Note: Files with Unknown type will return 0 (no icon) instead of folder icon
    auto iconIt = iconTextures.find(type);
    return (iconIt != iconTextures.end()) ? iconIt->second->GetID() : 0;
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

void ProjectPanel::RenderAssetItem(const std::filesystem::path& assetPath)
{
    std::string filename = assetPath.filename().string();
    bool isSelected = selectedItems.find(assetPath) != selectedItems.end();

    ImVec2 itemPos = ImGui::GetCursorPos();

    // First, render the selectable area (invisible but handles interaction)
    if (ImGui::Selectable(("##" + assetPath.string()).c_str(), isSelected,
        ImGuiSelectableFlags_AllowDoubleClick,
        ImVec2(kThumbnailSize, kThumbnailSize + 20.0f)))
    {
        if (ImGui::GetIO().KeyCtrl)
        {
            if (isSelected) selectedItems.erase(assetPath);
            else selectedItems.insert(assetPath);
        }
        else
        {
            selectedItems.clear();
            selectedItems.insert(assetPath);
        }

        if (ImGui::IsMouseDoubleClicked(0) && std::filesystem::is_directory(assetPath))
        {
            selectedFolder = assetPath;
            selectedItems.clear();
        }
    }

    // Context menu for this item
    if (ImGui::BeginPopupContextItem())
    {
        // Ensure this item is selected
        if (selectedItems.find(assetPath) == selectedItems.end())
        {
            selectedItems.clear();
            selectedItems.insert(assetPath);
        }

        if (ImGui::MenuItem("Rename", nullptr, false, selectedItems.size() == 1))
        {
            isEditing = true;
            justStartedEditing = true;
            editingPath = assetPath;
            std::string filename = assetPath.filename().string();
            strncpy_s(editBuffer, sizeof(editBuffer), filename.c_str(), sizeof(editBuffer) - 1);
            editBuffer[sizeof(editBuffer) - 1] = '\0';
        }

        if (ImGui::MenuItem("Delete"))
        {
            pendingDeletePaths = selectedItems;
            showDeleteModal = true;
        }

        ImGui::EndPopup();
    }

    // Drag & Drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        std::string relativePath = std::filesystem::relative(assetPath, "Assets").string();
        ImGui::SetDragDropPayload("ASSET_PAYLOAD", relativePath.c_str(), relativePath.size() + 1);
        ImGui::Text("%s", assetPath.filename().string().c_str());
        ImGui::EndDragDropSource();
    }

    ImGui::SetCursorPos(itemPos);
    
    GLuint textureID = GetIconOrThumbnail(assetPath);
    if (textureID != 0)
    {
        if (std::filesystem::is_directory(assetPath))
            ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(kThumbnailSize, kThumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
        else
            ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(kThumbnailSize, kThumbnailSize));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::Button(GetFileIcon(assetPath).c_str(), ImVec2(kThumbnailSize, kThumbnailSize));
        ImGui::PopStyleColor(3);
    }

    // Handle renaming
    if (isEditing && assetPath == editingPath)
    {
        if (justStartedEditing)
        {
            strcpy(editBuffer, filename.c_str());
            justStartedEditing = false;
        }
        
        ImGui::SetNextItemWidth(kThumbnailSize);
        ImGui::SetKeyboardFocusHere();
        
        if (ImGui::InputText("##rename", editBuffer, sizeof(editBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            std::string newName = editBuffer;
            if (!newName.empty() && newName != filename)
            {
                std::filesystem::path newPath = assetPath.parent_path() / newName;
                if (!std::filesystem::exists(newPath))
                {
                    try
                    {
                        std::filesystem::rename(assetPath, newPath);
                        if (selectedItems.count(assetPath))
                        {
                            selectedItems.erase(assetPath);
                            selectedItems.insert(newPath);
                        }
                        if (selectedFolder == assetPath)
                        {
                            selectedFolder = newPath;
                        }
                        lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                        renamedThisFrame.insert({newPath, ""});
                    }
                    catch (...) {}
                }
            }
            isEditing = false;
        }
        
        // End editing if item is deactivated
        if (ImGui::IsItemDeactivated())
        {
            isEditing = false;
        }
    }
    else
    {
        ImGui::TextWrapped(filename.c_str());
    }

    ImGui::NextColumn();
}

void ProjectPanel::HandleExternalFileDrop(const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists(filePath))
        return;

    // Determine destination path in Assets folder
    std::filesystem::path destPath;
    if (std::filesystem::is_directory(filePath))
    {
        // For directories, copy to selected folder
        destPath = selectedFolder / filePath.filename();
    }
    else
    {
        // For files, copy to selected folder
        destPath = selectedFolder / filePath.filename();
    }

    // Handle name conflicts
    int counter = 1;
    std::filesystem::path originalDest = destPath;
    while (std::filesystem::exists(destPath))
    {
        std::string stem = originalDest.stem().string();
        std::string ext = originalDest.extension().string();
        destPath = originalDest.parent_path() / (stem + " (" + std::to_string(counter) + ")" + ext);
        counter++;
    }

    try
    {
        if (std::filesystem::is_directory(filePath))
        {
            // Copy directory recursively
            std::filesystem::copy(filePath, destPath, std::filesystem::copy_options::recursive);
        }
        else
        {
            // Copy single file
            std::filesystem::copy_file(filePath, destPath);
        }

        // Refresh assets
        lastRefreshTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
    
    }
}

void ProjectPanel::CleanUp()
{
    thumbnailTextures.clear();
    iconTextures.clear();
    folderIconTexture.reset();
}
