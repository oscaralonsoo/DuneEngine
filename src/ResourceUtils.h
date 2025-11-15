#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <algorithm>

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

enum class ResourceFormat
{
    Binary,
    Text
};

class ResourceUtils
{
public:
    static ResourceType GetTypeFromExtension(const std::filesystem::path &path)
    {
        static const std::unordered_map<std::string, ResourceType> s_ExtensionMap = {
            {".png", ResourceType::Texture},
            {".jpg", ResourceType::Texture},
            {".jpeg", ResourceType::Texture},
            {".tga", ResourceType::Texture},
            {".bmp", ResourceType::Texture},
            {".dds", ResourceType::Texture},

            {".fbx", ResourceType::Model},
            {".obj", ResourceType::Model},
            {".gltf", ResourceType::Model},
            {".glb", ResourceType::Model},

            {".mat", ResourceType::Material},

            {".shader", ResourceType::Shader},

            {".prefab", ResourceType::Prefab}};

        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (auto it = s_ExtensionMap.find(ext); it != s_ExtensionMap.end())
            return it->second;

        return ResourceType::Unknown;
    }

    static std::string ToString(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Texture:
            return "Texture";
        case ResourceType::Model:
            return "Model";
        case ResourceType::Shader:
            return "Shader";
        case ResourceType::Material:
            return "Material";
        case ResourceType::Mesh:
            return "Mesh";
        case ResourceType::Prefab:
            return "Prefab";
        default:
            return "Unknown";
        }
    }

    static std::string GetInternalExtension(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Texture:
            return ".tex";
        case ResourceType::Model:
            return ".model";
        case ResourceType::Shader:
            return ".shader";
        case ResourceType::Material:
            return ".material";
        case ResourceType::Mesh:
            return ".mesh";
        case ResourceType::Prefab:
            return ".prefab";
        default:
            return ".unknown";
        }
    }

    static ResourceFormat GetDefaultFormat(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Shader:
            return ResourceFormat::Text;

        case ResourceType::Texture:
        case ResourceType::Model:
        case ResourceType::Material:
        case ResourceType::Mesh:
        case ResourceType::Prefab:
            return ResourceFormat::Binary;

        default:
            return ResourceFormat::Binary;
        }
    }
};
