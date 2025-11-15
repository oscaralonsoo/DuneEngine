#pragma once
#include "Module.h"

class ModuleImGui : public Module
{
public:
    ModuleImGui();
    ~ModuleImGui();

    bool Awake() override;
    bool Start() override;
    bool PreUpdate() override;
    bool Update() override;
    bool PostUpdate() override;
    bool CleanUp() override;

    bool showDemoWindow = true;
};