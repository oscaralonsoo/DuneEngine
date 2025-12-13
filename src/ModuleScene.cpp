#include "ModuleScene.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include "PrimitiveMesh.h"
#include "Model.h"
#include <glad/glad.h>
#include "Buffer.h"
#include "VertexArray.h"
#include "GameTime.h" 
#include "CameraComponent.h"
#include <glad/glad.h>
#include "Material.h"
#include <algorithm>

namespace
{
    std::shared_ptr<GameObject> CloneGameObjectRecursive(
        const std::shared_ptr<GameObject>& src,
        std::shared_ptr<GameObject> parent = nullptr)
        {
            auto dst = std::make_shared<GameObject>();
            dst->SetName(src->GetName());
            dst->SetSelected(false);
            
            // --- Transform ---
            if (auto* srcTransform = src->GetComponent<TransformComponent>())
            {
                auto* dstTransform = dst->GetComponent<TransformComponent>();
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
                auto& comp = dst->CreateComponent(ComponentType::Mesh);
                static_cast<MeshComponent&>(comp).SetMesh(srcMesh->GetMesh());
            }
            
            // --- Material ---
            if (auto* srcMat = src->GetComponent<MaterialComponent>())
            {
                auto& comp = dst->CreateComponent(ComponentType::Material);
                auto* dstMat = static_cast<MaterialComponent*>(&comp);
                
                auto srcMaterial = srcMat->GetMaterial();
                if (srcMaterial)
                {
                    auto clonedMaterial = std::make_shared<Material>(ResourceType::Material);
                    clonedMaterial->SetShader(srcMaterial->GetShader());
                    clonedMaterial->GetProperties()     = srcMaterial->GetProperties();
                    clonedMaterial->GetTextures()       = srcMaterial->GetTextures();
                    clonedMaterial->GetRenderSettings() = srcMaterial->GetRenderSettings();
                    dstMat->SetMaterial(clonedMaterial);
                }
            }
            
            // --- Camera ---
            if (auto* srcCam = src->GetComponent<CameraComponent>())
            {
                auto& comp = dst->CreateComponent(ComponentType::Camera);
                auto* dstCam = static_cast<CameraComponent*>(&comp);
                dstCam->SetFOV(srcCam->GetFOV());
                dstCam->SetNearClip(srcCam->GetNearClip());
                dstCam->SetFarClip(srcCam->GetFarClip());
            }
            
            // Parent
            if (parent)
            dst->SetParent(parent);
            
            // --- CLONAR HIJOS ---
            for (auto& child : src->GetChildren())
            {
                CloneGameObjectRecursive(child, dst);
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

    std::shared_ptr<GameObject> cameraGO = std::make_shared<GameObject>();
    cameraGO->SetName("MainCamera");

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

    return gameObject;
}

const std::vector<std::shared_ptr<GameObject>> ModuleScene::GetGameObjects()
{
    return mGameObjects;
}

void ModuleScene::SetSelected(std::shared_ptr<GameObject> gameObject)
{
    // Deselect
    if (selected)
    {
        selected->SetSelected(false);
    }

    selected = gameObject;
    
    if (gameObject)
    {
        gameObject->SetSelected(true);
    }
}

void ModuleScene::RemoveGameObject(std::shared_ptr<GameObject> gameObject)
{
    if (!gameObject) return;

    // If has parent, just delete itself
    if (gameObject->GetParent())
    {
        pendingDelete.push_back(gameObject);
        
        // Clear selection if the object being deleted is selected
        if (selected == gameObject)
        {
            selected = nullptr;
        }
    }
    else
    {
        // if doesn't have parent, delete all children too
        auto children = gameObject->GetAllDescendants();
        children.push_back(gameObject);

        for (auto& child : children)
        {
            pendingDelete.push_back(child);
            
            // Clear selection if any of the objects being deleted is selected
            if (selected == child)
            {
                selected = nullptr;
            }
        }
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
    mGameObjects.push_back(gameObject);
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

std::shared_ptr<GameObject> ModuleScene::CreateSphere()
{
    auto gameObject = CreateGameObjectWithName("Sphere");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateSphere());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreatePlane()
{
    auto gameObject = CreateGameObjectWithName("Plane");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreatePlane());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateCylinder()
{
    auto gameObject = CreateGameObjectWithName("Cylinder");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCylinder());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateCone()
{
    auto gameObject = CreateGameObjectWithName("Cone");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCone());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateTorus()
{
    auto gameObject = CreateGameObjectWithName("Torus");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateTorus());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateCapsule()
{
    auto gameObject = CreateGameObjectWithName("Capsule");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCapsule());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateQuad()
{
    auto gameObject = CreateGameObjectWithName("Quad");
    gameObject->CreateComponent(ComponentType::Mesh);
    gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateQuad());
    gameObject->CreateComponent(ComponentType::Material);
    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateCamera()
{
    auto gameObject = CreateGameObjectWithName("Camera");
    gameObject->CreateComponent(ComponentType::Camera);
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateEmptyGameObject()
{
    auto gameObject = CreateGameObjectWithName("GameObject");
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectFromModel(const std::filesystem::path& assetPath)
{
    auto gameObject = CreateGameObjectWithName(assetPath.stem().string());

    try
    {
        auto model = std::make_shared<Model>(assetPath);
        if (model)
        {
            // Filter meshes that have vertices and indices
            std::vector<std::shared_ptr<Mesh>> validMeshes;
            for (auto& mesh : model->GetMeshes())
            {
                if (!mesh->GetVertices().empty() && !mesh->GetIndices().empty())
                {
                    validMeshes.push_back(mesh);
                }
            }

            if (!validMeshes.empty())
            {
            if (validMeshes.size() > 1)
            {
                for (size_t i = 0; i < validMeshes.size(); ++i)
                {
                    auto childGo = CreateGameObject();
                    std::string childName = gameObject->GetName() + "_" + std::to_string(i);
                    childGo->SetName(GenerateUniqueName(childName));
                    childGo->CreateComponent(ComponentType::Mesh);
                    childGo->GetComponent<MeshComponent>()->SetMesh(validMeshes[i]);
                    childGo->CreateComponent(ComponentType::Material);
                    childGo->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                    childGo->SetParent(gameObject);

                    mGameObjects.push_back(childGo);
                }
            }
                else
                {
                    // Single valid mesh
                    gameObject->CreateComponent(ComponentType::Mesh);
                    gameObject->GetComponent<MeshComponent>()->SetMesh(validMeshes[0]);
                    gameObject->CreateComponent(ComponentType::Material);
                    gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
                }
            }
            else
            {
                gameObject->CreateComponent(ComponentType::Mesh);
                gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
                gameObject->CreateComponent(ComponentType::Material);
                gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
            }
        }
        else
        {
            gameObject->CreateComponent(ComponentType::Mesh);
            gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
            gameObject->CreateComponent(ComponentType::Material);
            gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
        }
    }
    catch (const std::exception& e)
    {
        // Exception occurred, use fallback
        gameObject->CreateComponent(ComponentType::Mesh);
        gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
        gameObject->CreateComponent(ComponentType::Material);
        gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    }
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectFromPrefab(const std::filesystem::path& assetPath)
{
    auto gameObject = CreateGameObjectWithName(assetPath.stem().string());
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent)
{
    auto duplicated = CreateGameObject();
    duplicated->SetName(GenerateUniqueName(original->GetName()));
    
    // --- Copy Transform ---
    if (auto* srcTransform = original->GetComponent<TransformComponent>())
    {
        auto* dstTransform = duplicated->GetComponent<TransformComponent>();
        if (dstTransform)
        {
            dstTransform->SetPosition(srcTransform->GetPosition());
            dstTransform->SetRotation(srcTransform->GetRotation());
            dstTransform->SetScale(srcTransform->GetScale());
        }
    }
    
    // --- Copy Mesh ---
    if (auto* srcMesh = original->GetComponent<MeshComponent>())
    {
        auto& comp = duplicated->CreateComponent(ComponentType::Mesh);
        static_cast<MeshComponent&>(comp).SetMesh(srcMesh->GetMesh());
    }
    
    // --- Copy Material ---
    if (auto* srcMat = original->GetComponent<MaterialComponent>())
    {
        auto& comp = duplicated->CreateComponent(ComponentType::Material);
        auto* dstMat = static_cast<MaterialComponent*>(&comp);
        
        auto srcMaterial = srcMat->GetMaterial();
        if (srcMaterial)
        {
            auto clonedMaterial = std::make_shared<Material>(ResourceType::Material);
            clonedMaterial->SetShader(srcMaterial->GetShader());
            clonedMaterial->GetProperties()     = srcMaterial->GetProperties();
            clonedMaterial->GetTextures()       = srcMaterial->GetTextures();
            clonedMaterial->GetRenderSettings() = srcMaterial->GetRenderSettings();
            dstMat->SetMaterial(clonedMaterial);
        }
    }
    
    // --- Copy Camera ---
    if (auto* srcCam = original->GetComponent<CameraComponent>())
    {
        auto& comp = duplicated->CreateComponent(ComponentType::Camera);
        auto* dstCam = static_cast<CameraComponent*>(&comp);
        dstCam->SetFOV(srcCam->GetFOV());
        dstCam->SetNearClip(srcCam->GetNearClip());
        dstCam->SetFarClip(srcCam->GetFarClip());
    }
    
    // Copy children recursively
    for (auto& child : original->GetChildren())
    {
        DuplicateGameObject(child, duplicated);
    }
    
    // Set parent
    duplicated->SetParent(parent);
    if (!duplicated->GetParent())
    {
        mGameObjects.push_back(duplicated);
    }
    return duplicated;
}

void ModuleScene::SaveInitialSnapshot()
{
    mInitialSnapshot.clear();

    for (const auto& go : mGameObjects)
    {
        if (!go->GetParent())
        {
            mInitialSnapshot.push_back(CloneGameObjectRecursive(go));
        }
    }

    mHasSnapshot = true;
}

void ModuleScene::RestoreSnapshot()
{
    if (!mHasSnapshot)
        return;

    selected = nullptr;
    mGameObjects.clear();

    for (const auto& root : mInitialSnapshot)
    {
        auto clonedRoot = CloneGameObjectRecursive(root);
        mGameObjects.push_back(clonedRoot);

        auto descendants = clonedRoot->GetAllDescendants();
        for (auto& child : descendants)
            mGameObjects.push_back(child);
    }

    RebuildQuadtree();
}


void ModuleScene::RebuildQuadtree()
{
    if (!mQuadtree)
        return;

    // Calcular AABB global de la escena
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

    for (const auto& go : mGameObjects)
    {
        AABB worldBox;
        if (ComputeWorldAABB(go, worldBox))
        {
            mQuadtree->Insert(go, worldBox);
        }
    }
}

void ModuleScene::AddGameObject(std::shared_ptr<GameObject> go)
{
    if (!go) return;

    // Evitar duplicados
    auto it = std::find(mGameObjects.begin(), mGameObjects.end(), go);
    if (it == mGameObjects.end())
    {
        mGameObjects.push_back(go);
    }
}