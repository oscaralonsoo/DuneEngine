#include "ToolbarPanel.h"
#include "Engine.h"
#include "ModuleScene.h"

void ToolbarPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 40.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags toolbarFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Toolbar", nullptr, toolbarFlags))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

        bool isPlaying = GameTime::IsPlaying();
        bool isPaused  = GameTime::IsPaused();

        auto* scene = Engine::GetInstance().scene.get();

        // === PLAY / PAUSE ===
        if (!isPlaying)
        {
            if (ImGui::Button("Play"))
            {
                scene->SaveInitialSnapshot();
                GameTime::Play();
            }
        }
        else
        {
            if (ImGui::Button("Pause"))
            {
                GameTime::Pause();
            }
        }

        ImGui::SameLine();

        // === STOP ===
        if (ImGui::Button("Stop"))
        {
            GameTime::Stop();
            scene->RestoreSnapshot();
        }

        ImGui::SameLine();

        // === STEP ===
        ImGui::BeginDisabled(!isPaused);
        if (ImGui::Button("Step"))
        {
            GameTime::StepOneFrame();
        }
        ImGui::EndDisabled();

        ImGui::SameLine(0.0f, 30.0f);

        // TimeScale
        float timeScale = GameTime::GetTimeScale();
        ImGui::Text("Speed");
        ImGui::SameLine();
        if (ImGui::SliderFloat("##TimeScale", &timeScale, 0.0f, 4.0f, "%.2f"))
        {
            GameTime::SetTimeScale(timeScale);
        }

        ImGui::SameLine(0.0f, 30.0f);
        ImGui::Text("GameTime: %.2f", GameTime::GetGameTime());
        ImGui::SameLine();
        ImGui::Text("RealTime: %.2f", GameTime::GetRealTimeSinceStartup());
    }

    ImGui::End();
    ImGui::PopStyleVar(1);
}
