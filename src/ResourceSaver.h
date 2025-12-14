#pragma once

#include "TextureImportData.h"

struct MeshImportData;

class ResourceSaver
{
public:
    ResourceSaver() = default;
    ~ResourceSaver() = default;

    static void Save(const ImportData &importData);
    static bool SaveMesh(const MeshImportData& data, const std::string& path);
    static void SaveBinary(const ImportData& importData);
    static void SaveMeta(const ImportData& importData);
};

std::ostream& operator<<(std::ostream& os, const MeshImportData& data);

std::ostream &operator<<(std::ostream &os, const TextureImportData &texture);
