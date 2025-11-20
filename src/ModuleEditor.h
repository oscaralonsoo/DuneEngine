#pragma once
#include "Module.h"
#include <vector>
#include <memory>
#include <imgui.h>

class EditorPanel
{
public:
    virtual ~EditorPanel() = default;

    virtual bool Start() { return true; }

    virtual void OnImGuiRender() = 0;

    virtual void CleanUp() {}
};

class Panel : public EditorPanel
{
public:
    Panel(const char* name)
        : panelName(name)
        , open(true)
    {}

    virtual ~Panel() = default;

    void Render()
    {
        if (!open)
            return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 pos = viewport->WorkPos;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = windowFlags;

        if (ImGui::Begin(panelName.c_str(), &open, flags))
        {
            OnImGuiRender();   // contenido específico del panel
        }

        ImGui::End();
    }

    bool IsOpen() const { return open; }
    void SetOpen(bool value) { open = value; }

protected:
    // Función que implementan los hijos con el contenido del panel
    virtual void OnImGuiRender() = 0;

    std::string       panelName;
    bool              open = true;
    ImGuiWindowFlags  windowFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking;
};

class ModuleEditor : public Module
{
public:
    ModuleEditor();
    ~ModuleEditor() = default;

    bool Start() override;
    bool Update() override;
    bool CleanUp() override;

    template<typename T, typename... Args>
    T* AddPanel(Args&&... args)
    {
        panels.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T*>(panels.back().get());
    }

private:
    std::vector<std::unique_ptr<EditorPanel>> panels;
};
