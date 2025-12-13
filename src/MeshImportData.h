#pragma once

#include "ImportData.h"
#include "Mesh.h"
#include "Material.h"

struct MeshImportData : public ImportData
{
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Material> material;
    AABB aabb;

    MeshImportData() : ImportData(ResourceType::Mesh) {}
};