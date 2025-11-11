#include "Engine.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleTextures.h"
#include "Globals.h"
#include <iostream>

Engine::Engine()
{
    window = std::make_shared<ModuleWindow>();
    input = std::make_shared<ModuleInput>();
    render = std::make_shared<ModuleRender>();
    camera = std::make_shared<ModuleCamera>();
    textures = std::make_shared<ModuleTextures>();

    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(input));
    AddModule(std::static_pointer_cast<Module>(render));
    AddModule(std::static_pointer_cast<Module>(camera));
    AddModule(std::static_pointer_cast<Module>(textures));
}

Engine &Engine::GetInstance()
{
    static Engine instance;
    return instance;
}

void Engine::AddModule(std::shared_ptr<Module> module)
{
    module->Init();
    modules.push_back(module);
}

bool Engine::Awake()
{
    bool result = true;
    for (const auto &module : modules)
    {
        result = module.get()->Awake();
        if (!result)
            break;
    }

    return result;
}

bool Engine::Start()
{
    bool result = true;
    for (const auto &module : modules)
    {
        result = module.get()->Start();
        if (!result)
        {
            break;
        }
    }

    return result;
}

bool Engine::Update()
{

    bool ret = true;

    if (input->GetWindowEvent(WE_QUIT) == true)
        ret = false;

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
    for (const auto &module : modules)
    {
        result = module.get()->PreUpdate();
        if (!result)
        {
            break;
        }
    }

    return result;
}

bool Engine::DoUpdate()
{
    bool result = true;
    for (const auto &module : modules)
    {
        result = module.get()->Update();
        if (!result)
        {
            break;
        }
    }

    return result;
}

bool Engine::PostUpdate()
{
    bool result = true;
    for (const auto &module : modules)
    {
        result = module.get()->PostUpdate();
        if (!result)
        {
            break;
        }
    }

    return result;
}

bool Engine::CleanUp()
{
    bool result = true;
    for (auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        result = (*it).get()->CleanUp();
        if (!result)
        {
            break;
        }
    }

    return result;
}

int Engine::Run()
{
    EngineState state = EngineState::CREATE;
    int result = EXIT_FAILURE;

    while (state != EngineState::EXIT)
    {
        switch (state)
        {
        case EngineState::CREATE:
            state = EngineState::AWAKE;
            break;

        case EngineState::AWAKE:
            if (Awake())
                state = EngineState::START;
            else
                state = EngineState::FAIL;
            break;

        case EngineState::START:
            if (Start())
                state = EngineState::LOOP;
            else
                state = EngineState::FAIL;
            break;

        case EngineState::LOOP:
            if (!Update())
                state = EngineState::CLEAN;
            break;

        case EngineState::CLEAN:
            if (CleanUp())
            {
                result = EXIT_SUCCESS;
                state = EngineState::EXIT;
            }
            else
                state = EngineState::FAIL;
            break;

        case EngineState::FAIL:
            result = EXIT_FAILURE;
            state = EngineState::EXIT;
            break;
        }
    }

    return result;
}
