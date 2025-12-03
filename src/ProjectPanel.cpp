#include "ProjectPanel.h"
#include "Engine.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>

bool ProjectPanel::Start()
{
    return true;
}

void ProjectPanel::OnImGuiRender()
{
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

    std::filesystem::path assetsPath = "Assets";
    if (std::filesystem::exists(assetsPath))
    {
        RenderDirectoryTree(assetsPath, assetsPath);
    }

    ImGui::End();
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

            bool nodeOpen = ImGui::TreeNodeEx(filename.c_str(), flags);

            if (nodeOpen)
            {
                RenderDirectoryTree(entryPath, basePath);
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Selectable(filename.c_str());

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                std::string relativePath = std::filesystem::relative(entryPath, basePath).string();
                ImGui::SetDragDropPayload("ASSET_PAYLOAD", relativePath.c_str(), relativePath.size() + 1);
                ImGui::Text("Dragging %s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}

void ProjectPanel::CleanUp()
{
}
