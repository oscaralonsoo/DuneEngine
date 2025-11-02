#include "Engine.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "Globals.h"
#include <iostream>

Engine::Engine()
{
    window = std::make_shared<ModuleWindow>();
    render = std::make_shared<ModuleRender>();

    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(render));

}

Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

void Engine::AddModule(std::shared_ptr<Module> module){
    module->Init();
    modules.push_back(module);
}


bool Engine::Awake()
{
    bool result = true;
    for (const auto& module : modules) {
        result =  module.get()->Awake();
        if (!result) break;
		
    }

    return result;
}

bool Engine::Start()
{
    bool result = true;
    for (const auto& module : modules) {
        result = module.get()->Start();
        if (!result) {
            break;
        }
    }

    return result;
}

bool Engine::Update()
{

    bool ret = true;

    // if (input->GetWindowEvent(WE_QUIT) == true)
    //     ret = false;

    if (ret == true)
        ret = PreUpdate();

    if (ret == true)
        ret = DoUpdate();

    if (ret == true)
        ret = PostUpdate();

    return ret;
}

bool Engine::PreUpdate()
{
    bool result = true;
    for (const auto& module : modules) {
        result = module.get()->PreUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

bool Engine::DoUpdate()
{
    bool result = true;
    for (const auto& module : modules) {
        result = module.get()->Update();
        if (!result) {
            break;
        }
    }

    return result;
}

bool Engine::PostUpdate()
{
    bool result = true;
    for (const auto& module : modules) {
        result = module.get()->PostUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}


bool Engine::CleanUp()
{
    bool result = true;
    for (const auto& module : modules) {
        result = module.get()->CleanUp();
        if (!result) {
            break;
        }
    }

    return result;
}