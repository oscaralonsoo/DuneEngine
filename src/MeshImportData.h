#pragma once

#include "ImportData.h"
#include <vector>

struct MeshImportData : ImportData
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    MeshImportData() : ImportData(ResourceType::Mesh) {}
};
