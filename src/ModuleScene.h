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

    std::shared_ptr<GameObject> CreateGameObject();
    const std::vector<std::shared_ptr<GameObject>> GetGameObjects();

    Raycaster* GetRaycaster() const { return raycaster; }
    void SetSelected(std::shared_ptr<GameObject> go);
    void ResetSelecteds();
    std::shared_ptr<GameObject> GetSelected() const { return selected; }
private:
    std::shared_ptr<GameObject> root;
    std::shared_ptr<GameObject> test;
    std::vector<std::shared_ptr<GameObject>> mGameObjects;
    Raycaster *raycaster;
    std::shared_ptr<GameObject> selected;
};
