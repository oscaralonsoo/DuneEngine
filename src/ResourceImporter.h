#pragma once

#include "Texture.h"
#include "TextureImportData.h"
#include "MeshImportData.h"

class ResourceImporter
{
public:
    ResourceImporter() = default;
    ~ResourceImporter() = default;

    static TextureImportData Import(const std::filesystem::path &path, TextureImportData importData);
    static MeshImportData Import(const std::filesystem::path &path, MeshImportData importData);
};

