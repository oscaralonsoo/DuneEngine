#pragma once

#include "TextureImportData.h"

class ResourceSaver
{
public:
    ResourceSaver() = default;
    ~ResourceSaver() = default;

    static void Save(const TextureImportData &importData);
    static void SaveBinary(const TextureImportData& importData);
    static void SaveMeta(const TextureImportData& importData);
};

std::ostream &operator<<(std::ostream &os, const TextureImportData &texture);
