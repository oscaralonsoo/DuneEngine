#pragma once

#include <filesystem>

enum class ResourceType
{
    Unknown = 0,
    Texture,
    Model,
    Shader,
    Material,
    Mesh,
    Prefab
};

class Resource
{
public:
    Resource(ResourceType type = ResourceType::Unknown)
        : mType(type) {}
    ~Resource() = default;

    const std::string &GetName() const { return mName; }
    void SetName(const std::string &name) { mName = name; }

    const std::filesystem::path &GetPath() { return mFilePath; }
    void SetPath(const std::filesystem::path &path) { mFilePath = path; }

    ResourceType GetType() const { return mType; }

    //void SetUUID(UUID uuid) { m_UUID = uuid; }
    //UUID GetUUID() const { return m_UUID; }
    
protected:
    std::string mName;
    ResourceType mType;
    std::filesystem::path mFilePath;
    // UUID m_UUID; --FIXME: add UUID class
};