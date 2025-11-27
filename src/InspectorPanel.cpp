#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ModuleInput.h"
#include <imgui.h>
#include <algorithm>

bool InspectorPanel::Start()
{
    return true;
}

void InspectorPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    // Size & position
    float panelWidth  = workSize.x * InspectorPanel::kDefaultFraction;
    float panelHeight = workSize.y;
    float maxAllowed = workSize.x - (workSize.x * InspectorPanel::kDefaultFraction) - InspectorPanel::kMinCenterWidth;
    panelWidth = std::clamp(panelWidth, InspectorPanel::kMinPanelWidth, maxAllowed);

    // Anchor to right
    ImVec2 pos = ImVec2(workPos.x + workSize.x - panelWidth, workPos.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Inspector", nullptr, flags);

    // Handle external drag-drop source
    const std::string& draggedFile = ModuleInput::GetDraggedFile();
    if (!draggedFile.empty())
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
        {
            ImGui::SetDragDropPayload("FILE_PATH", draggedFile.c_str(), draggedFile.size() + 1);
            ImGui::Text("Dragging: %s", draggedFile.c_str());
            ImGui::EndDragDropSource();
        }
    }

    auto scene = Engine::GetInstance().scene;
    if (scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        if (selected)
        {
            ImGui::Text("Name: %s", selected->GetName().c_str());
            ImGui::Separator();

            // Automatically detect and render all components
            const auto& components = selected->GetComponents();
            for (const auto& component : components)
            {
                component->OnInspectorRender(panelWidth);
                ImGui::Separator();
            }
        }
        else
        {
            ImGui::TextDisabled("No selection");
        }
    }

    ImGui::End();
}

void InspectorPanel::CleanUp()
{

}




 