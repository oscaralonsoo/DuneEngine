#pragma once

#include "ImportData.h"
#include "Vertex.h"

#include <vector>

struct MeshImportData : public ImportData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    MeshImportData() : ImportData(ResourceType::Mesh) {}
};
