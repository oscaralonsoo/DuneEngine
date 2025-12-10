#include "ModuleScene.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include "PrimitiveMesh.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "GameTime.h" 
#include <glad/glad.h>

namespace
{
    std::shared_ptr<GameObject> CloneGameObject(const std::shared_ptr<GameObject>& src)
    {
        auto dst = std::make_shared<GameObject>();
        dst->SetName(src->GetName());
        dst->SetSelected(false); // nunca arrancamos con selección

        // --- Transform ---
        if (auto* srcTransform = src->GetComponent<TransformComponent>())
        {
            auto& dstTransformComp = dst->CreateComponent(ComponentType::Transform);
            auto* dstTransform = dynamic_cast<TransformComponent*>(&dstTransformComp);

            if (dstTransform)
            {
                // Copia profunda de TRS  🔥
                dstTransform->SetPosition(srcTransform->GetPosition());
                dstTransform->SetRotation(srcTransform->GetRotation());
                dstTransform->SetScale(srcTransform->GetScale());
            }
        }

        // --- Mesh ---
        if (auto* srcMesh = src->GetComponent<MeshComponent>())
        {
            auto& dstMeshComp = dst->CreateComponent(ComponentType::Mesh);
            auto* dstMesh = dynamic_cast<MeshComponent*>(&dstMeshComp);
            if (dstMesh)
            {
                dstMesh->SetMesh(srcMesh->GetMesh());
            }
        }

        // --- Material ---
        if (auto* srcMat = src->GetComponent<MaterialComponent>())
        {
            auto& dstMatComp = dst->CreateComponent(ComponentType::Material);
            auto* dstMat = dynamic_cast<MaterialComponent*>(&dstMatComp);
            if (dstMat)
            {
                dstMat->SetMaterial(srcMat->GetMaterial());
            }
        }

        return dst;
    }
}


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
    if (!GameTime::IsPlaying() && !GameTime::IsStepFrame())
        return true;

    for (auto& go : mGameObjects)
    {
        if (!go->Update())
            return false;
    }

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

void ModuleScene::SaveInitialSnapshot()
{
    if (mHasSnapshot)
        return; // solo guardamos una vez antes del primer Play

    mInitialSnapshot.clear();
    mInitialSnapshot.reserve(mGameObjects.size());

    for (const auto& go : mGameObjects)
    {
        mInitialSnapshot.push_back(CloneGameObject(go));
    }

    mHasSnapshot = true;
}

void ModuleScene::RestoreSnapshot()
{
    if (!mHasSnapshot)
        return;

    // Limpiar selección
    selected = nullptr;

    // Sustituimos la escena actual por el snapshot
    mGameObjects.clear();

    for (const auto& savedGo : mInitialSnapshot)
    {
        mGameObjects.push_back(CloneGameObject(savedGo));
    }
}
