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
    bool PostUpdate() override;
    bool CleanUp() override;

    std::shared_ptr<GameObject> CreateGameObject();
    const std::vector<std::shared_ptr<GameObject>> GetGameObjects();

    Raycaster* GetRaycaster() const { return raycaster; }
    void SetSelected(std::shared_ptr<GameObject> go);
    void ResetSelecteds();
    std::shared_ptr<GameObject> GetSelected() const { return selected; }
    void RemoveGameObject(std::shared_ptr<GameObject> go);
    void ProcessPendingDeletes();

    // Utility methods for hierarchy management
    std::string GenerateUniqueName(const std::string& baseName);
    std::shared_ptr<GameObject> CreateGameObjectWithName(const std::string& name);
    std::shared_ptr<GameObject> CreateCube();
    std::shared_ptr<GameObject> CreateGameObjectFromModel(const std::filesystem::path& assetPath);
    std::shared_ptr<GameObject> DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent = nullptr);
private:
    std::shared_ptr<GameObject> root;
    std::shared_ptr<GameObject> test;
    std::vector<std::shared_ptr<GameObject>> mGameObjects;
    Raycaster *raycaster;
    std::shared_ptr<GameObject> selected;
    std::vector<std::shared_ptr<GameObject>> pendingDelete;
};
