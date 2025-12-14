#include "ModuleResource.h"
#include "ResourceImporter.h"
#include "ResourceSaver.h"
#include "ResourceLoader.h"
#include "Texture.h"
#include "Model.h"
#include "ResourceUtils.h"
#include "Globals.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TextureImportData.h"
#include "ModelImportData.h"
#include "ImportData.h"

#include "IL/il.h"

bool ModuleResource::Awake()
{
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    return true;
}

bool ModuleResource::Start()
{

    return true;
}

std::shared_ptr<Resource> ModuleResource::RequestResource(std::filesystem::path assetPath)
{
    ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);
    
    // For models (FBX, OBJ), load directly without meta/binary system
    if (type == ResourceType::Model)
    {
        // Check if we already have this model loaded by path
        for (const auto& [key, resource] : mResources)
        {
            if (resource->GetAssetPath() == assetPath)
                return resource;
        }
        
        // Create new model and cache it
        auto model = std::make_shared<Model>(assetPath);
        if (model)
        {
            model->SetAssetPath(assetPath);
            mResources[model->GetUID().ToString()] = model;
            return model;
        }
        return nullptr;
    }
    
    // For other resources, use the meta/binary system
    std::filesystem::path metaPath = assetPath;
    metaPath += ".meta";

    if (!std::filesystem::exists(metaPath))
        return Find(ImportFile(assetPath));

    UID uid = ResourceUtils::GetUIDFromMeta(metaPath);

    if (auto resource = Find(uid))
        return resource;

    std::string binPath = ResourceUtils::GetLibraryPath(uid);
    if (!binPath.empty())
    {
        ResourceType type = ResourceUtils::GetTypeFromExtension(binPath);
        switch (type)
        {
        case ResourceType::Texture:
        {
            return CreateResource(ResourceLoader::LoadTexture(uid));
        }
        case ResourceType::Model:
        {
            return CreateResource(ResourceLoader::LoadModel(uid));
        }
        default:
            SDL_Log("Unknown resource type %d", static_cast<int>(type));
            return nullptr;
        }
    }
    else
    {
        return Find(ImportFile(assetPath));
    }
}

UID ModuleResource::ImportFile(std::filesystem::path assetPath)
{
    UID uid = UID();

    switch (ResourceUtils::GetTypeFromExtension(assetPath))
    {
    case ResourceType::Texture:
    {
        TextureImportData importData;
        importData.assetPath = assetPath;
        importData.uid = uid;
        importData.type = ResourceType::Texture;

        importData = ResourceImporter::Import(assetPath, importData);

        ResourceSaver::Save(importData);

        CreateResource(importData);
        break;
    }
    case ResourceType::Model:
        ModelImportData importData;
        importData.assetPath = assetPath;
        importData.uid = uid;
        importData.type = ResourceType::Model;

        importData = ResourceImporter::Import(assetPath, importData);

        ResourceSaver::Save(importData);

        CreateResource(importData);
        break;
    }

    return uid;
}

std::shared_ptr<Resource> ModuleResource::Find(UID uid)
{
    auto it = mResources.find(uid.ToString());
    if (it != mResources.end())
    {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<Resource> ModuleResource::CreateResource(const ImportData &importData)
{
    std::shared_ptr<Resource> resource = nullptr;

    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        auto textureData = dynamic_cast<const TextureImportData *>(&importData);
        resource = std::make_shared<Texture>(*textureData);

        break;
    }
    case ResourceType::Model:
    {
        auto modelData = dynamic_cast<const ModelImportData *>(&importData);
        resource = std::make_shared<Model>(*modelData);

        break;
    }
    default:
        break;
    }

    if (resource)
    {
        mResources[importData.uid.ToString()] = resource;
    }

    return resource;
}

int ModuleResource::GetResourceUsageCount(UID resourceUID) const
{
    int count = 0;
    
    auto scene = Engine::GetInstance().scene;
    if (!scene)
        return 0;
    
    auto gameObjects = scene->GetGameObjects();
    
    for (const auto& go : gameObjects)
    {
        if (!go)
            continue;
        
        // Check MeshComponent
        if (auto meshComp = go->GetComponent<MeshComponent>())
        {
            if (auto mesh = meshComp->GetMesh())
            {
                if (mesh->GetUID() == resourceUID)
                    count++;
            }
        }
        
        // Check MaterialComponent
        if (auto matComp = go->GetComponent<MaterialComponent>())
        {
            if (auto material = matComp->GetMaterial())
            {
                // Check if material matches
                if (material->GetUID() == resourceUID)
                {
                    count++;
                }
                else
                {
                    // Check textures material
                    const auto& textures = material->GetTextures();
                    if (textures.albedo && textures.albedo->GetUID() == resourceUID)
                        count++;
                    if (textures.normal && textures.normal->GetUID() == resourceUID)
                        count++;
                    if (textures.metallic && textures.metallic->GetUID() == resourceUID)
                        count++;
                    if (textures.roughness && textures.roughness->GetUID() == resourceUID)
                        count++;
                    if (textures.ao && textures.ao->GetUID() == resourceUID)
                        count++;
                    if (textures.emissive && textures.emissive->GetUID() == resourceUID)
                        count++;
                }
            }
        }
    }
    
    return count;
}
