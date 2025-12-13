#pragma once

#include "UID.h"
#include <filesystem>

enum class ResourceType
{
    Unknown = 0,
    Texture,
    Model,
    Shader,
    Material,
    Mesh,
    Cubemap,
    Prefab
};

class Resource
{
public:
    Resource(ResourceType type = ResourceType::Unknown)
        : mType(type) {}
    virtual ~Resource() = default;

    const std::string &GetName() const { return mName; }
    void SetName(const std::string &name) { mName = name; }

    const std::filesystem::path &GetAssetPath() { return mAssetPath; }
    void SetAssetPath(const std::filesystem::path &assetPath) { mAssetPath = assetPath; }

    const std::filesystem::path &GetLibraryPath() { return mLibraryPath; }
    void SetLibraryPath(const std::filesystem::path &libraryPath) { mLibraryPath = libraryPath; }

    ResourceType GetType() const { return mType; }

    void SetUID(UID uid) { mUID = uid; }
    UID GetUID() const { return mUID; }

protected:
    std::string mName;
    ResourceType mType;
    std::filesystem::path mAssetPath;
    std::filesystem::path mLibraryPath;
    UID mUID;
};