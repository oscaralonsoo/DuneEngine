#pragma once

#include "ImportData.h"
#include "Vertex.h"
#include "AABB.h"

#include <vector>

struct MeshImportData : public ImportData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    AABB aabb;
    
    MeshImportData() : ImportData(ResourceType::Mesh) {}
};
