#include "ModuleResource.h"
#include "ResourceImporter.h"
#include "ResourceSaver.h"
#include "ResourceLoader.h"
#include "Texture.h"
#include "Model.h"
#include "ResourceUtils.h"
#include "Globals.h"
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