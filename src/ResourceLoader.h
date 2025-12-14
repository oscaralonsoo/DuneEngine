#pragma once

#include "TextureImportData.h"
#include "ModelImportData.h"
#include "MeshImportData.h"

struct MeshImportData;

class ResourceLoader
{
public:
    ResourceLoader() = default;
    ~ResourceLoader() = default;

    static TextureImportData LoadTexture(const UID uid);
    static ModelImportData LoadModel(const UID uid);
};

std::istream &operator>>(std::istream &is, ModelImportData &data);

std::istream &operator>>(std::istream &is, TextureImportData &texture);

std::istream &operator>>(std::istream &is, MeshImportData &mesh);