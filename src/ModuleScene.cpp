#include "ModuleScene.h"

ModuleScene::ModuleScene()
{
    name = "scene";
}

bool ModuleScene::Start()
{
    root = new GameObject();
    root->SetName("Root");
    mGameObjects.push_back(root);

    return true;
}

bool ModuleScene::Update()
{
    return true;
}

bool ModuleScene::CleanUp()
{
    return true;
}

GameObject* ModuleScene::CreateGameObject(const std::string& name)
{
    GameObject* go = new GameObject();
    go->SetName(name);
    mGameObjects.push_back(go);
    return go;
}

const std::vector<GameObject *> ModuleScene::GetGameObjects()
{
    return mGameObjects;
}
