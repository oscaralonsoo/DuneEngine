#include "ModuleScene.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include "PrimitiveMesh.h"
#include "Buffer.h"
#include "VertexArray.h"
#include <glad/glad.h>

ModuleScene::ModuleScene()
{
    name = "scene";
}

bool ModuleScene::Start()
{
    root = std::make_shared<GameObject>();
    root->SetName("Root");
    mGameObjects.push_back(root);
    root->CreateComponent(ComponentType::Mesh);
    root->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
    root->CreateComponent(ComponentType::Material);
    root->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));

    raycaster = new Raycaster();

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

std::shared_ptr<GameObject> ModuleScene::CreateGameObject()
{
    std::shared_ptr<GameObject> go = std::make_shared<GameObject>();
    mGameObjects.push_back(go);
    return go;
}

const std::vector<std::shared_ptr<GameObject>> ModuleScene::GetGameObjects()
{
    return mGameObjects;
}

void ModuleScene::SetSelected(std::shared_ptr<GameObject> go)
{
    selected = go;
    go->SetSelected(!go->IsSelected());
}

void ModuleScene::ResetSelecteds()
{
    selected = nullptr;
    
    for (auto& go : mGameObjects)
    {
        go->SetSelected(false);
    }
}