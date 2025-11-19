#pragma once
#include <vector>
#include "Module.h"
#include "GameObject.h"
#include "Raycaster.h"

class GameObject;
class Component;

class ModuleScene : public Module
{
public:
    ModuleScene();
    ~ModuleScene() = default;

    bool Start() override;
    bool Update() override;
    bool CleanUp() override;

    GameObject* CreateGameObject(const std::string& name);
    const std::vector<GameObject *> GetGameObjects();

    Raycaster* GetRaycaster() const { return raycaster; }
private:
    GameObject *root = nullptr;
    std::vector<GameObject *> mGameObjects;

    Raycaster *raycaster;
};
