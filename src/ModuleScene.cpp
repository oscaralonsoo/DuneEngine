#include "ModuleScene.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include "PrimitiveMesh.h"
#include "Model.h"
#include <glad/glad.h>

ModuleScene::ModuleScene()
{
    name = "scene";
}

bool ModuleScene::Start()
{
    root = std::make_shared<GameObject>(); //Create Root
    root->SetName("Root");
    mGameObjects.push_back(root);

    // Add root Components 
    root->CreateComponent(ComponentType::Mesh);
    root->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());

    root->CreateComponent(ComponentType::Material);
    root->GetComponent<MaterialComponent>()->SetMaterial(
        std::make_shared<Material>(ResourceType::Material)
    );

    raycaster = new Raycaster();

    return true;
}

bool ModuleScene::Update()
{
    return true;
}

bool ModuleScene::PostUpdate()
{
    if (!pendingDelete.empty())
    {
        ProcessPendingDeletes();
    }
    return true;
}

bool ModuleScene::CleanUp()
{
    return true;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObject()
{
    auto gameObject = std::make_shared<GameObject>();
    mGameObjects.push_back(gameObject);

    return gameObject;
}

const std::vector<std::shared_ptr<GameObject>> ModuleScene::GetGameObjects()
{
    return mGameObjects;
}

void ModuleScene::SetSelected(std::shared_ptr<GameObject> gameObject)
{
    if (!gameObject) return;

    selected = gameObject;
    gameObject->SetSelected(!gameObject->IsSelected());
}

void ModuleScene::ResetSelecteds()
{
    selected = nullptr;

    for (auto& gameObject : mGameObjects)
    {
        if (gameObject->IsSelected())
        {
            gameObject->SetSelected(false);
        }
    }
}

void ModuleScene::RemoveGameObject(std::shared_ptr<GameObject> gameObject)
{
    if (!gameObject) return;

    // If has parent, just delete itself
    if (gameObject->GetParent())
    {
        pendingDelete.push_back(gameObject);
    }
    else
    {
        // if doesn't have parent, delete all children too
        auto children = gameObject->GetAllDescendants();
        children.push_back(gameObject);

        for (auto& child : children)
        {
            pendingDelete.push_back(child);
        }
    }
    
    if (selected == gameObject)
    {
        selected = nullptr;
    }
}

void ModuleScene::ProcessPendingDeletes()
{
    for (auto& gameObject : pendingDelete)
    {
        // remove from parent
        if (auto parent = gameObject->GetParent())
        {
            parent->RemoveChild(gameObject);
        }

        // Remove from scene
        for (size_t i = 0; i < mGameObjects.size(); i++)
        {
            if (mGameObjects[i] == gameObject)
            {
                mGameObjects.erase(mGameObjects.begin() + i);
                break;
            }
        }
    }

    pendingDelete.clear();
}

std::string ModuleScene::GenerateUniqueName(const std::string& baseName)
{
    std::string candidate = baseName;
    int counter = 1;

    while (true)
    {
        bool nameExists = false;
        for (auto& existingGameObject : mGameObjects)
        {
            if (existingGameObject->GetName() == candidate)
            {
                nameExists = true;
                break;
            }
        }
        if (!nameExists)
            break;

        candidate = baseName + "_" + std::to_string(counter);
        counter++;
    }
    return candidate;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectWithName(const std::string& name)
{
    auto gameObject = CreateGameObject();
    gameObject->SetName(GenerateUniqueName(name));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateCube()
{
    auto gameObject = CreateGameObjectWithName("Cube");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectFromModel(const std::filesystem::path& assetPath)
{
    auto gameObject = CreateGameObjectWithName(assetPath.stem().string());
    gameObject->CreateComponent(ComponentType::Transform);

    auto model = Model::Load(assetPath);
    if (model && !model->GetMeshes().empty())
    {
        if (model->GetMeshes().size() > 1)
        {
            for (size_t i = 0; i < model->GetMeshes().size(); ++i)
            {
                auto childGo = CreateGameObject();
                std::string childName = gameObject->GetName() + "_" + std::to_string(i);
                childGo->SetName(GenerateUniqueName(childName));
                childGo->CreateComponent(ComponentType::Transform);
                childGo->CreateComponent(ComponentType::Mesh);
                childGo->GetComponent<MeshComponent>()->SetMesh(model->GetMeshes()[i]);
                childGo->CreateComponent(ComponentType::Material);
                childGo->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                childGo->SetParent(gameObject);
            }
        }
        else
        {
            // Single mesh
            gameObject->CreateComponent(ComponentType::Mesh);
            gameObject->GetComponent<MeshComponent>()->SetMesh(model->GetMeshes()[0]);
            gameObject->CreateComponent(ComponentType::Material);
            gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
        }
    }
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent)
{
    auto duplicated = CreateGameObject();
    duplicated->SetName(GenerateUniqueName(original->GetName()));
    // Copy components
    for (auto& comp : original->GetComponents())
    {
        duplicated->CreateComponent(comp->GetType());
    }
    // Copy children recursively
    for (auto& child : original->GetChildren())
    {
        DuplicateGameObject(child, duplicated);
    }
    // Set parent
    duplicated->SetParent(parent);
    return duplicated;
}
