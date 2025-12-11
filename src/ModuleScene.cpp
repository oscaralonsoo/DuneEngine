#include "ModuleScene.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include "PrimitiveMesh.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "GameTime.h" 
#include "CameraComponent.h"
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
            // Reutilizar el Transform que GameObject ya crea en su constructor
            auto* dstTransform = dst->GetComponent<TransformComponent>();

            // Por si algún día tienes GameObjects sin Transform, aseguramos
            if (!dstTransform)
            {
                auto& dstTransformComp = dst->CreateComponent(ComponentType::Transform);
                dstTransform = dynamic_cast<TransformComponent*>(&dstTransformComp);
            }

            if (dstTransform)
            {
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
        if (auto* srcCam = src->GetComponent<CameraComponent>())
        {
            auto& dstCamComp = dst->CreateComponent(ComponentType::Camera);
            auto* dstCam = dynamic_cast<CameraComponent*>(&dstCamComp);
            if (dstCam)
            {
                dstCam->SetFOV(srcCam->GetFOV());
                dstCam->SetNearClip(srcCam->GetNearClip());
                dstCam->SetFarClip(srcCam->GetFarClip());
            }
        }

        return dst;
    }

    bool ComputeWorldAABB(const std::shared_ptr<GameObject>& go, AABB& outBox)
    {
        MeshComponent* meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp)
            return false;

        std::shared_ptr<Mesh> mesh = meshComp->GetMesh();
        if (!mesh)
            return false;

        const AABB& localBox = mesh->GetAABB();

        glm::mat4 world = glm::mat4(1.0f);
        if (auto* tc = go->GetComponent<TransformComponent>())
        {
            world = tc->GetWorldTransform();
        }

        outBox = TransformAABB(localBox, world);
        return true;
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

    std::shared_ptr<GameObject> cameraGO = std::make_shared<GameObject>();
    cameraGO->SetName("Main Camera");

    auto* camTransform = cameraGO->GetComponent<TransformComponent>();
    camTransform->SetPosition({ 0.0f, 1.0f, 5.0f });   
    camTransform->SetRotation({ 0.0f, 0.0f, 0.0f });

    cameraGO->CreateComponent(ComponentType::Camera);

    mGameObjects.push_back(cameraGO);

    AABB initialBounds(glm::vec3(-1000.0f), glm::vec3(1000.0f));
    mQuadtree = std::make_unique<Quadtree>(initialBounds, 6, 4);
    RebuildQuadtree();

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
    ResetSelecteds();

    selected = go;

    if (go)
        go->SetSelected(true);
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

    RebuildQuadtree();
}

void ModuleScene::RebuildQuadtree()
{
    if (!mQuadtree)
        return;

    // Calcular AABB global de la escena (solo objetos con Mesh)
    AABB sceneBounds;
    bool hasAny = false;

    for (const auto& go : mGameObjects)
    {
        AABB worldBox;
        if (ComputeWorldAABB(go, worldBox))
        {
            if (!hasAny)
            {
                sceneBounds = worldBox;
                hasAny = true;
            }
            else
            {
                sceneBounds.Encapsulate(worldBox);
            }
        }
    }

    if (!hasAny)
        return; // no hay nada que meter en el quadtree

    // Resetear el árbol con estos límites
    mQuadtree->SetBounds(sceneBounds);

    // Insertar todos los estáticos (aquí, todos los que tengan Mesh)
    for (const auto& go : mGameObjects)
    {
        AABB worldBox;
        if (ComputeWorldAABB(go, worldBox))
        {
            mQuadtree->Insert(go, worldBox);
        }
    }
}
