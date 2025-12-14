#include "ModuleScene.h"
#include "MeshComponent.h"
#include "Engine.h"
#include "ModuleResource.h"
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
#include <fstream>
#include <unordered_map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "ModuleInput.h"

namespace
{
    std::shared_ptr<GameObject> CloneGameObjectRecursive(
        const std::shared_ptr<GameObject> &src,
        std::shared_ptr<GameObject> parent = nullptr)
    {
        auto dst = std::make_shared<GameObject>();
        dst->SetName(src->GetName());
        dst->SetSelected(false);

        // --- Transform ---
        if (auto *srcTransform = src->GetComponent<TransformComponent>())
        {
            auto *dstTransform = dst->GetComponent<TransformComponent>();
            if (dstTransform)
            {
                dstTransform->SetPosition(srcTransform->GetPosition());
                dstTransform->SetRotation(srcTransform->GetRotation());
                dstTransform->SetScale(srcTransform->GetScale());
            }
        }

        // --- Mesh ---
        if (auto *srcMesh = src->GetComponent<MeshComponent>())
        {
            auto &comp = dst->CreateComponent(ComponentType::Mesh);
            static_cast<MeshComponent &>(comp).SetMesh(srcMesh->GetMesh());
        }

        // --- Material ---
        if (auto *srcMat = src->GetComponent<MaterialComponent>())
        {
            auto &comp = dst->CreateComponent(ComponentType::Material);
            auto *dstMat = static_cast<MaterialComponent *>(&comp);

            auto srcMaterial = srcMat->GetMaterial();
            if (srcMaterial)
            {
                auto clonedMaterial = std::make_shared<Material>(ResourceType::Material);
                clonedMaterial->SetShader(srcMaterial->GetShader());
                clonedMaterial->GetProperties() = srcMaterial->GetProperties();
                clonedMaterial->GetTextures() = srcMaterial->GetTextures();
                clonedMaterial->GetRenderSettings() = srcMaterial->GetRenderSettings();
                dstMat->SetMaterial(clonedMaterial);
            }
        }

        // --- Camera ---
        if (auto *srcCam = src->GetComponent<CameraComponent>())
        {
            auto &comp = dst->CreateComponent(ComponentType::Camera);
            auto *dstCam = static_cast<CameraComponent *>(&comp);
            dstCam->SetFOV(srcCam->GetFOV());
            dstCam->SetNearClip(srcCam->GetNearClip());
            dstCam->SetFarClip(srcCam->GetFarClip());
        }

        // Parent
        if (parent)
            dst->SetParent(parent);

        // --- CLONAR HIJOS ---
        for (auto &child : src->GetChildren())
        {
            CloneGameObjectRecursive(child, dst);
        }

        return dst;
    }

    bool ComputeWorldAABB(const std::shared_ptr<GameObject> &go, AABB &outBox)
    {
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp)
            return false;

        std::shared_ptr<Mesh> mesh = meshComp->GetMesh();
        if (!mesh)
            return false;

        const AABB &localBox = mesh->GetAABB();

        glm::mat4 world = glm::mat4(1.0f);
        if (auto *tc = go->GetComponent<TransformComponent>())
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
    raycaster = new Raycaster();

    std::shared_ptr<GameObject> cameraGO = std::make_shared<GameObject>();
    cameraGO->SetName("MainCamera");

    auto *camTransform = cameraGO->GetComponent<TransformComponent>();
    camTransform->SetPosition({0.0f, 1.0f, 5.0f});
    camTransform->SetRotation({0.0f, 0.0f, 0.0f});

    cameraGO->CreateComponent(ComponentType::Camera);

    mGameObjects.push_back(cameraGO);

    auto streetGO = CreateGameObjectFromModel(
        "Assets/objects/Street/street2.FBX"
    );
    mGameObjects.push_back(streetGO);

    if (auto* transform = streetGO->GetComponent<TransformComponent>())
    {
        transform->SetRotation({-90.0f, 0.0f, 0.0f});
        transform->SetScale({0.1f, 0.1f, 0.1f});
        glm::vec3 pos = transform->GetPosition();
        pos.y -= 5.0f;        
        transform->SetPosition(pos);
    }

    AABB initialBounds(glm::vec3(-1000.0f), glm::vec3(1000.0f));
    mQuadtree = std::make_unique<Quadtree>(initialBounds, 6, 4);
    RebuildQuadtree();

    return true;
}

bool ModuleScene::Update()
{
    if (!GameTime::IsPlaying() && !GameTime::IsStepFrame())
        return true;

    for (auto &go : mGameObjects)
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
    if (!gameObject)
        return;

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

        for (auto &child : children)
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
    for (auto &gameObject : pendingDelete)
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

    // Rebuild quadtree after deleting objects to remove stale references
    RebuildQuadtree();
}

std::string ModuleScene::GenerateUniqueName(const std::string &baseName)
{
    std::string candidate = baseName;
    int counter = 1;

    while (true)
    {
        bool nameExists = false;
        for (auto &existingGameObject : mGameObjects)
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

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectWithName(const std::string &name)
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

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectFromModel(const std::filesystem::path &assetPath)
{
    auto gameObject = CreateGameObjectWithName(assetPath.stem().string());

    try
    {
        std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;
        std::shared_ptr<Resource> resource = resourceManager->RequestResource(assetPath);
        std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(resource);
        if (model)
        {
            // Filter meshes that have vertices and indices
            std::vector<std::shared_ptr<Mesh>> validMeshes;
            for (auto &mesh : model->GetMeshes())
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
    catch (const std::exception &e)
    {
        // Exception occurred, use fallback
        gameObject->CreateComponent(ComponentType::Mesh);
        gameObject->GetComponent<MeshComponent>()->SetMesh(PrimitiveMesh::CreateCube());
        gameObject->CreateComponent(ComponentType::Material);
        gameObject->GetComponent<MaterialComponent>()->SetMaterial(std::make_shared<Material>(ResourceType::Material));
    }
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::CreateGameObjectFromPrefab(const std::filesystem::path &assetPath)
{
    auto gameObject = CreateGameObjectWithName(assetPath.stem().string());
    return gameObject;
}

std::shared_ptr<GameObject> ModuleScene::DuplicateGameObject(std::shared_ptr<GameObject> original, std::shared_ptr<GameObject> parent)
{
    auto duplicated = CreateGameObject();
    duplicated->SetName(GenerateUniqueName(original->GetName()));

    // --- Copy Transform ---
    if (auto *srcTransform = original->GetComponent<TransformComponent>())
    {
        auto *dstTransform = duplicated->GetComponent<TransformComponent>();
        if (dstTransform)
        {
            dstTransform->SetPosition(srcTransform->GetPosition());
            dstTransform->SetRotation(srcTransform->GetRotation());
            dstTransform->SetScale(srcTransform->GetScale());
        }
    }

    // --- Copy Mesh ---
    if (auto *srcMesh = original->GetComponent<MeshComponent>())
    {
        auto &comp = duplicated->CreateComponent(ComponentType::Mesh);
        static_cast<MeshComponent &>(comp).SetMesh(srcMesh->GetMesh());
    }

    // --- Copy Material ---
    if (auto *srcMat = original->GetComponent<MaterialComponent>())
    {
        auto &comp = duplicated->CreateComponent(ComponentType::Material);
        auto *dstMat = static_cast<MaterialComponent *>(&comp);

        auto srcMaterial = srcMat->GetMaterial();
        if (srcMaterial)
        {
            auto clonedMaterial = std::make_shared<Material>(ResourceType::Material);
            clonedMaterial->SetShader(srcMaterial->GetShader());
            clonedMaterial->GetProperties() = srcMaterial->GetProperties();
            clonedMaterial->GetTextures() = srcMaterial->GetTextures();
            clonedMaterial->GetRenderSettings() = srcMaterial->GetRenderSettings();
            dstMat->SetMaterial(clonedMaterial);
        }
    }

    // --- Copy Camera ---
    if (auto *srcCam = original->GetComponent<CameraComponent>())
    {
        auto &comp = duplicated->CreateComponent(ComponentType::Camera);
        auto *dstCam = static_cast<CameraComponent *>(&comp);
        dstCam->SetFOV(srcCam->GetFOV());
        dstCam->SetNearClip(srcCam->GetNearClip());
        dstCam->SetFarClip(srcCam->GetFarClip());
    }

    // Copy children recursively
    for (auto &child : original->GetChildren())
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

    for (const auto &go : mGameObjects)
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

    for (const auto &root : mInitialSnapshot)
    {
        auto clonedRoot = CloneGameObjectRecursive(root);
        mGameObjects.push_back(clonedRoot);

        auto descendants = clonedRoot->GetAllDescendants();
        for (auto &child : descendants)
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

    for (const auto &go : mGameObjects)
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

    for (const auto &go : mGameObjects)
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
    if (!go)
        return;

    // Evitar duplicados
    auto it = std::find(mGameObjects.begin(), mGameObjects.end(), go);
    if (it == mGameObjects.end())
    {
        mGameObjects.push_back(go);
    }
}

void ModuleScene::SaveScene(const std::string &path)
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

    rapidjson::Value gameObjects(rapidjson::kArrayType);

    for (const auto &go : mGameObjects)
    {
        if (!go)
            continue;

        rapidjson::Value goJson(rapidjson::kObjectType);

        // UID
        goJson.AddMember(
            "UID",
            rapidjson::Value(go->GetUID().ToString().c_str(), allocator),
            allocator);

        // Parent UID
        if (auto parent = go->GetParent())
        {
            goJson.AddMember(
                "ParentUID",
                rapidjson::Value(parent->GetUID().ToString().c_str(), allocator),
                allocator);
        }
        else
        {
            goJson.AddMember("ParentUID", "", allocator);
        }

        // Name
        goJson.AddMember(
            "Name",
            rapidjson::Value(go->GetName().c_str(), allocator),
            allocator);

        // Transform
        if (auto *transform = go->GetComponent<TransformComponent>())
        {
            rapidjson::Value pos(rapidjson::kArrayType);
            rapidjson::Value rot(rapidjson::kArrayType);
            rapidjson::Value scl(rapidjson::kArrayType);

            const auto &p = transform->GetPosition();
            const auto &r = transform->GetRotation();
            const auto &s = transform->GetScale();

            pos.PushBack(p.x, allocator).PushBack(p.y, allocator).PushBack(p.z, allocator);
            rot.PushBack(r.x, allocator).PushBack(r.y, allocator).PushBack(r.z, allocator);
            scl.PushBack(s.x, allocator).PushBack(s.y, allocator).PushBack(s.z, allocator);

            goJson.AddMember("Position", pos, allocator);
            goJson.AddMember("Rotation", rot, allocator);
            goJson.AddMember("Scale", scl, allocator);
        }

        if (auto *materialComp = go->GetComponent<MaterialComponent>())
        {
            rapidjson::Value materialJson(rapidjson::kObjectType);

            auto material = materialComp->GetMaterial();
            if (material)
            {
                const auto &props = material->GetProperties();
                rapidjson::Value color(rapidjson::kArrayType);
                color.PushBack(props.color.r, allocator)
                    .PushBack(props.color.g, allocator)
                    .PushBack(props.color.b, allocator)
                    .PushBack(props.color.a, allocator);

                rapidjson::Value emissive(rapidjson::kArrayType);
                emissive.PushBack(props.emissive.r, allocator)
                    .PushBack(props.emissive.g, allocator)
                    .PushBack(props.emissive.b, allocator);

                materialJson.AddMember("Color", color, allocator);
                materialJson.AddMember("Metallic", props.metallic, allocator);
                materialJson.AddMember("Roughness", props.roughness, allocator);
                materialJson.AddMember("AO", props.ao, allocator);
                materialJson.AddMember("Emissive", emissive, allocator);

                const auto &textures = material->GetTextures();

                if (textures.albedo)
                    materialJson.AddMember("Albedo", rapidjson::Value(textures.albedo->GetName().c_str(), allocator), allocator);
                if (textures.normal)
                    materialJson.AddMember("Normal", rapidjson::Value(textures.normal->GetName().c_str(), allocator), allocator);
                if (textures.metallic)
                    materialJson.AddMember("MetallicTex", rapidjson::Value(textures.metallic->GetName().c_str(), allocator), allocator);
                if (textures.roughness)
                    materialJson.AddMember("RoughnessTex", rapidjson::Value(textures.roughness->GetName().c_str(), allocator), allocator);
                if (textures.ao)
                    materialJson.AddMember("AOTex", rapidjson::Value(textures.ao->GetName().c_str(), allocator), allocator);
                if (textures.emissive)
                    materialJson.AddMember("EmissiveTex", rapidjson::Value(textures.emissive->GetName().c_str(), allocator), allocator);

                // Guardar RenderSettings
                const auto &renderSettings = material->GetRenderSettings();
                materialJson.AddMember("TransparencyMode", static_cast<int>(renderSettings.transparencyMode), allocator);
                materialJson.AddMember("AlphaCutoff", renderSettings.alphaCutoff, allocator);
                materialJson.AddMember("BlendMode", static_cast<int>(renderSettings.blendMode), allocator);
                materialJson.AddMember("CullMode", static_cast<int>(renderSettings.cullMode), allocator);
                materialJson.AddMember("DepthTest", renderSettings.depthTest, allocator);
                materialJson.AddMember("Wireframe", renderSettings.wireframe, allocator);
            }

            goJson.AddMember("MaterialComponent", materialJson, allocator);
        }

        gameObjects.PushBack(goJson, allocator);
    }

    doc.AddMember("GameObjects", gameObjects, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream file(path);
    if (file.is_open())
    {
        file << buffer.GetString();
        file.close();
    }
}

void ModuleScene::LoadScene(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return;

    std::string jsonStr(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (!doc.HasMember("GameObjects") || !doc["GameObjects"].IsArray())
        return;

    // Clear current scene
    selected = nullptr;
    mGameObjects.clear();
    pendingDelete.clear();

    std::unordered_map<std::string, std::shared_ptr<GameObject>> goMap;

    const auto &arr = doc["GameObjects"];

    std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;

    // --- First pass: create GameObjects ---
    for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
    {
        const auto &goJson = arr[i];

        auto go = std::make_shared<GameObject>();

        // UID
        UID uid(goJson["UID"].GetString());
        go->SetUID(uid);

        // Name
        go->SetName(goJson["Name"].GetString());

        // Transform
        if (auto *transform = go->GetComponent<TransformComponent>())
        {
            const auto &pos = goJson["Position"];
            const auto &rot = goJson["Rotation"];
            const auto &scl = goJson["Scale"];

            transform->SetPosition({pos[0].GetFloat(),
                                    pos[1].GetFloat(),
                                    pos[2].GetFloat()});

            transform->SetRotation({rot[0].GetFloat(),
                                    rot[1].GetFloat(),
                                    rot[2].GetFloat()});

            transform->SetScale({scl[0].GetFloat(),
                                 scl[1].GetFloat(),
                                 scl[2].GetFloat()});
        }

        if (goJson.HasMember("MaterialComponent") && goJson["MaterialComponent"].IsObject())
        {
            go->CreateComponent(ComponentType::Material);
            MaterialComponent *materialComponent = go->GetComponent<MaterialComponent>();

            const auto &matJson = goJson["MaterialComponent"];
            auto material = std::make_shared<Material>();

            // Propiedades PBR
            if (matJson.HasMember("Color"))
            {
                const auto &color = matJson["Color"];
                material->GetProperties().color = {
                    color[0].GetFloat(),
                    color[1].GetFloat(),
                    color[2].GetFloat(),
                    color[3].GetFloat()};
            }
            if (matJson.HasMember("Metallic"))
                material->GetProperties().metallic = matJson["Metallic"].GetFloat();
            if (matJson.HasMember("Roughness"))
                material->GetProperties().roughness = matJson["Roughness"].GetFloat();
            if (matJson.HasMember("AO"))
                material->GetProperties().ao = matJson["AO"].GetFloat();
            if (matJson.HasMember("Emissive"))
            {
                const auto &emissive = matJson["Emissive"];
                material->GetProperties().emissive = {
                    emissive[0].GetFloat(),
                    emissive[1].GetFloat(),
                    emissive[2].GetFloat()};
            }

            // Texturas
            auto &textures = material->GetTextures();
            if (matJson.HasMember("Albedo"))
                textures.albedo = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["Albedo"].GetString()));
            if (matJson.HasMember("Normal"))
                textures.normal = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["Normal"].GetString()));
            if (matJson.HasMember("MetallicTex"))
                textures.metallic = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["MetallicTex"].GetString()));
            if (matJson.HasMember("RoughnessTex"))
                textures.roughness = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["RoughnessTex"].GetString()));
            if (matJson.HasMember("AOTex"))
                textures.ao = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["AOTex"].GetString()));
            if (matJson.HasMember("EmissiveTex"))
                textures.emissive = std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource(matJson["EmissiveTex"].GetString()));

            auto &renderSettings = material->GetRenderSettings();
            if (matJson.HasMember("TransparencyMode"))
                renderSettings.transparencyMode = static_cast<MaterialRenderSettings::TransparencyMode>(matJson["TransparencyMode"].GetInt());
            if (matJson.HasMember("AlphaCutoff"))
                renderSettings.alphaCutoff = matJson["AlphaCutoff"].GetFloat();
            if (matJson.HasMember("BlendMode"))
                renderSettings.blendMode = static_cast<MaterialRenderSettings::BlendMode>(matJson["BlendMode"].GetInt());
            if (matJson.HasMember("CullMode"))
                renderSettings.cullMode = static_cast<MaterialRenderSettings::CullMode>(matJson["CullMode"].GetInt());
            if (matJson.HasMember("DepthTest"))
                renderSettings.depthTest = matJson["DepthTest"].GetBool();
            if (matJson.HasMember("Wireframe"))
                renderSettings.wireframe = matJson["Wireframe"].GetBool();

            materialComponent->SetMaterial(material);
        }

        mGameObjects.push_back(go);
        goMap[uid.ToString()] = go;
    }

    // --- Second pass: rebuild hierarchy ---
    for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
    {
        const auto &goJson = arr[i];
        const std::string parentUID = goJson["ParentUID"].GetString();

        if (!parentUID.empty())
        {
            auto child = mGameObjects[i];
            auto parent = goMap[parentUID];

            if (parent)
                child->SetParent(parent);
        }
    }

    RebuildQuadtree();
}