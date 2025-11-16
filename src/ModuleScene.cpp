#include "ModuleScene.h"

ModuleScene::ModuleScene()
{
    name = "scene";
}

bool ModuleScene::Start()
{
    root = new GameObject();

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
