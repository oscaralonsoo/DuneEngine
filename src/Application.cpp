#include "Application.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "Globals.h"
#include <iostream>

Application::Application()
{
    modules.emplace_back(std::make_unique<ModuleWindow>(this));
    // modules.emplace_back(std::make_unique<ModuleRenderer>(this));
    // modules.emplace_back(std::make_unique<ModuleImGui>(this));
}

Application::~Application()
{
    CleanUp();
}

bool Application::Init()
{
    for (auto& module : modules)
    {
        if (!module->Awake())
        {
            LOG_INFO("Failed to awake module: %s", module->GetName());
            return false;
        }
    }

    for (auto& module : modules)
    {
        if (!module->Start())
        {
            LOG_INFO("Failed to start module: %s", module->GetName());
            return false;
        }
    }

    return true;
}

void Application::Run()
{
    if (!Init()) return;

    while (running)
    {
        for (auto& module : modules)
            module->PreUpdate();

        for (auto& module : modules)
            module->Update();

        for (auto& module : modules)
            module->PostUpdate();
    }

    CleanUp();
}

void Application::CleanUp()
{
    for (auto it = modules.rbegin(); it != modules.rend(); ++it)
        (*it)->CleanUp();
}

template<typename T>
T* Application::GetModule()
{
    for (auto& module : modules)
    {
        if (auto ptr = dynamic_cast<T*>(module.get()))
            return ptr;
    }
    return nullptr;
}
