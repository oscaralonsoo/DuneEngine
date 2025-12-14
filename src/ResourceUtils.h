#pragma once

#include "Resource.h"
#include "ResourceFormat.h"

#include <string>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <fstream>

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
            {".tex", ResourceType::Texture},

            {".mesh", ResourceType::Mesh},

            {".fbx", ResourceType::Model},
            {".obj", ResourceType::Model},
            {".gltf", ResourceType::Model},
            {".glb", ResourceType::Model},
            {".model", ResourceType::Model},

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
            return ResourceFormat::JSON;

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

    static UID GetUIDFromMeta(const std::filesystem::path &metaPath)
    {
        std::ifstream file(metaPath);

        std::string line;
        std::getline(file, line);

        auto uidPos = line.find("uid=\"");

        uidPos += 5;
        auto endPos = line.find("\"", uidPos);

        std::string uidStr = line.substr(uidPos, endPos - uidPos);

        return UID(uidStr);
    }

    static std::string GetLibraryPath(const UID &uid)
    {
        const std::filesystem::path libraryDir = "Library";
        const std::string uidStr = uid.ToString();

        if (!std::filesystem::exists(libraryDir))
            return "";

        for (const auto &entry : std::filesystem::directory_iterator(libraryDir))
        {
            if (!entry.is_regular_file())
                continue;

            const std::filesystem::path &path = entry.path();

            if (path.stem() == uidStr)
            {
                return path.string();
            }
        }

        return "";
    }
};
