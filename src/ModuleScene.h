#pragma once
#include <vector>
#include "Module.h"
#include "GameObject.h"

class ModuleScene : public Module
{
public:
    ModuleScene();
    ~ModuleScene() = default;

    bool Start() override;
    bool Update() override;
    bool CleanUp() override;

    GameObject* CreateGameObject(const std::string& name);

private:
    GameObject *root = nullptr;
    std::vector<GameObject *> mGameObjects;
};
