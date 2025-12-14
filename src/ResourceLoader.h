#pragma once

#include "TextureImportData.h"

struct MeshImportData;

class ResourceLoader
{
public:
    ResourceLoader() = default;
    ~ResourceLoader() = default;

    static TextureImportData Load(const UID uid);
    static MeshImportData LoadMesh(const UID uid);
    static void Unload(const ImportData data);
};

std::istream& operator>>(std::istream& is, MeshImportData& data);

std::istream &operator>>(std::istream &is, TextureImportData &texture);