#pragma once

#include "TextureImportData.h"

class ResourceSaver
{
public:
    ResourceSaver() = default;
    ~ResourceSaver() = default;

    static void Save(const ImportData &importData);
    static void SaveBinary(const ImportData& importData);
    static void SaveMeta(const ImportData& importData);
};

std::ostream &operator<<(std::ostream &os, const TextureImportData &texture);
