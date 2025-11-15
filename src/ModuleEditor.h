#pragma once
#include "Module.h"
#include <vector>
#include <memory>
#include <imgui.h>

class EditorPanel
{
public:
    virtual ~EditorPanel() = default;
    virtual void OnImGuiRender() = 0;
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

    void DrawDockspace();
};
