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
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Inspector", nullptr, flags);

    auto scene = Engine::GetInstance().scene;
    if (scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        if (selected)
        {
            static char nameBuffer[256];
            strcpy(nameBuffer, selected->GetName().c_str());
            ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                selected->SetName(nameBuffer);
            }
            ImGui::Separator();

            // Automatically detect and render all components
            const auto& components = selected->GetComponents();
            for (const auto& component : components)
            {
                component->OnInspectorRender(ImGui::GetContentRegionAvail().x);
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




 