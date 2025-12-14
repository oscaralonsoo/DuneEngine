#pragma once
#include <vector>
#include "Module.h"
#include "GameObject.h"
#include "Raycaster.h"
#include "Quadtree.h"

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
    Quadtree* GetQuadtree() const { return mQuadtree.get(); }
    void SetSelected(std::shared_ptr<GameObject> go);
    std::shared_ptr<GameObject> GetSelected() const { return selected; }
    void RemoveGameObject(std::shared_ptr<GameObject> go);
    void ProcessPendingDeletes();

    // Utility methods for hierarchy management
    std::string GenerateUniqueName(const std::string& baseName);
    std::shared_ptr<GameObject> CreateGameObjectWithName(const std::string& name);
    std::shared_ptr<GameObject> CreateEmptyGameObject();
    std::shared_ptr<GameObject> CreateCube();
    std::shared_ptr<GameObject> CreateSphere();
    std::shared_ptr<GameObject> CreatePlane();
    std::shared_ptr<GameObject> CreateCylinder();
    std::shared_ptr<GameObject> CreateCone();
    std::shared_ptr<GameObject> CreateTorus();
    std::shared_ptr<GameObject> CreateCapsule();
    std::shared_ptr<GameObject> CreateQuad();
    std::shared_ptr<GameObject> CreateCamera();
    std::shared_ptr<GameObject> CreateGameObjectFromModel(const std::filesystem::path& assetPath);
    std::shared_ptr<GameObject> CreateGameObjectFromPrefab(const std::filesystem::path& assetPath);
    std::shared_ptr<GameObject> DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent = nullptr);

    void AddGameObject(std::shared_ptr<GameObject> go);

    void SaveInitialSnapshot();

    void RestoreSnapshot();

    void RebuildQuadtree();

    void SaveScene(const std::string& path);
    void LoadScene(const std::string& path);
private:
    std::shared_ptr<GameObject> root;
    std::shared_ptr<GameObject> test;
    std::vector<std::shared_ptr<GameObject>> mGameObjects;
    Raycaster *raycaster;
    std::shared_ptr<GameObject> selected;
    std::vector<std::shared_ptr<GameObject>> pendingDelete;

    std::vector<std::shared_ptr<GameObject>> mInitialSnapshot;
    bool mHasSnapshot = false;
    std::unique_ptr<Quadtree> mQuadtree;
};
