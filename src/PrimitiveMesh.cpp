#include "PrimitiveMesh.h"
#include "Mesh.h"
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>

std::shared_ptr<Mesh> PrimitiveMesh::CreateCube(const glm::vec3 &size, int subdivideW, int subdivideH, int subdivideD)
{
    std::vector<Vertex> vertices(24);

    // Front
    vertices[0].Position = {-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[0].Normals = {0.0f, 0.0f, 1.0f};
    vertices[0].TexCoords = {0.0f, 0.0f};

    vertices[1].Position = {size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[1].Normals = {0.0f, 0.0f, 1.0f};
    vertices[1].TexCoords = {1.0f, 0.0f};

    vertices[2].Position = {size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[2].Normals = {0.0f, 0.0f, 1.0f};
    vertices[2].TexCoords = {1.0f, 1.0f};

    vertices[3].Position = {-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[3].Normals = {0.0f, 0.0f, 1.0f};
    vertices[3].TexCoords = {0.0f, 1.0f};

    // Back
    vertices[4].Position = {size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[4].Normals = {0.0f, 0.0f, -1.0f};
    vertices[4].TexCoords = {0.0f, 0.0f};

    vertices[5].Position = {-size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[5].Normals = {0.0f, 0.0f, -1.0f};
    vertices[5].TexCoords = {1.0f, 0.0f};

    vertices[6].Position = {-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[6].Normals = {0.0f, 0.0f, -1.0f};
    vertices[6].TexCoords = {1.0f, 1.0f};

    vertices[7].Position = {size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[7].Normals = {0.0f, 0.0f, -1.0f};
    vertices[7].TexCoords = {0.0f, 1.0f};

    // Top

    vertices[8].Position = {-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[8].Normals = {0.0f, 1.0f, 0.0f};
    vertices[8].TexCoords = {0.0f, 0.0f};

    vertices[9].Position = {size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[9].Normals = {0.0f, 1.0f, 0.0f};
    vertices[9].TexCoords = {1.0f, 0.0f};

    vertices[10].Position = {size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[10].Normals = {0.0f, 1.0f, 0.0f};
    vertices[10].TexCoords = {1.0f, 1.0f};

    vertices[11].Position = {-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[11].Normals = {0.0f, 1.0f, 0.0f};
    vertices[11].TexCoords = {0.0f, 1.0f};

    // Bottom
    vertices[12].Position = {size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[12].Normals = {0.0f, -1.0f, 0.0f};
    vertices[12].TexCoords = {0.0f, 0.0f};

    vertices[13].Position = {-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[13].Normals = {0.0f, -1.0f, 0.0f};
    vertices[13].TexCoords = {1.0f, 0.0f};

    vertices[14].Position = {-size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[14].Normals = {0.0f, -1.0f, 0.0f};
    vertices[14].TexCoords = {1.0f, 1.0f};

    vertices[15].Position = {size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[15].Normals = {0.0f, -1.0f, 0.0f};
    vertices[15].TexCoords = {0.0f, 1.0f};

    // Right
    vertices[16].Position = {size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[16].Normals = {1.0f, 0.0f, 0.0f};
    vertices[16].TexCoords = {0.0f, 0.0f};

    vertices[17].Position = {size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[17].Normals = {1.0f, 0.0f, 0.0f};
    vertices[17].TexCoords = {1.0f, 0.0f};

    vertices[18].Position = {size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[18].Normals = {1.0f, 0.0f, 0.0f};
    vertices[18].TexCoords = {1.0f, 1.0f};

    vertices[19].Position = {size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[19].Normals = {1.0f, 0.0f, 0.0f};
    vertices[19].TexCoords = {0.0f, 1.0f};

    // Left
    vertices[20].Position = {-size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[20].Normals = {-1.0f, 0.0f, 0.0f};
    vertices[20].TexCoords = {0.0f, 0.0f};

    vertices[21].Position = {-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[21].Normals = {-1.0f, 0.0f, 0.0f};
    vertices[21].TexCoords = {1.0f, 0.0f};

    vertices[22].Position = {-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[22].Normals = {-1.0f, 0.0f, 0.0f};
    vertices[22].TexCoords = {1.0f, 1.0f};

    vertices[23].Position = {-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[23].Normals = {-1.0f, 0.0f, 0.0f};
    vertices[23].TexCoords = {0.0f, 1.0f};

    std::vector<uint32_t> indices = {
        0,
        1,
        2,
        2,
        3,
        0, // Front
        4,
        5,
        6,
        6,
        7,
        4, // Back
        8,
        9,
        10,
        10,
        11,
        8, // Top
        12,
        13,
        14,
        14,
        15,
        12, // Bottom
        16,
        17,
        18,
        18,
        19,
        16, // Right
        20,
        21,
        22,
        22,
        23,
        20, // Left
    };

    const std::shared_ptr<Mesh> &cubeMesh = std::make_shared<Mesh>(vertices, indices);
    cubeMesh->SetName("Cube");

    AABB cubeAABB(glm::vec3(-size.x * 0.5f, -size.y * 0.5f, -size.z * 0.5f), glm::vec3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
    cubeMesh->SetAABB(cubeAABB);

    return cubeMesh;
}

std::shared_ptr<Mesh> PrimitiveMesh::CreateSphere(float radius, float height, int radialSegments, int rings, bool isHemiSphere)
{

    int i, j, prevrow, thisrow, point = 0;
    float x, y, z;

    std::vector<Vertex> data;
    std::vector<uint32_t> indices;

    float scale = height * (isHemiSphere ? 1.0f : 0.5f);

    thisrow = 0;
    prevrow = 0;
    for (j = 0; j <= rings; j++)
    {
        float v = static_cast<float>(j) / rings;
        float w = glm::sin(glm::pi<float>() * v);
        y = scale * glm::cos(glm::pi<float>() * v);

        for (i = 0; i <= radialSegments; i++)
        {
            float u = static_cast<float>(i) / radialSegments;

            x = glm::sin(u * glm::two_pi<float>());
            z = glm::cos(u * glm::two_pi<float>());

            Vertex vertex;
            if (isHemiSphere && y < 0.0f)
            {
                glm::vec3 p = glm::vec3(x * radius * w, 0.0f, z * radius * w);
                vertex.Position = p;
                vertex.Normals = glm::vec3(0.0f, -1.0f, 0.0f);
                vertex.Tangent = glm::vec4(z, 0.0f, -x, 1.0f);
                vertex.TexCoords = glm::vec2(u, v);
                data.emplace_back(vertex);
            }
            else
            {
                glm::vec3 p = glm::vec3(x * radius * w, y, z * radius * w);
                glm::vec3 normal = glm::vec3(x * w, y / scale, z * w);

                vertex.Position = p;
                vertex.Normals = glm::normalize(normal);
                vertex.Tangent = glm::vec4(z, 0.0f, -x, 1.0f);
                vertex.TexCoords = glm::vec2(u, v);
                data.emplace_back(vertex);
            }

            point++;

            if (i > 0 && j > 0)
            {
                indices.push_back(prevrow + i - 1);
                indices.push_back(thisrow + i - 1);
                indices.push_back(prevrow + i);

                indices.push_back(prevrow + i);
                indices.push_back(thisrow + i - 1);
                indices.push_back(thisrow + i);
            }
        }

        prevrow = thisrow;
        thisrow = point;
    }

    const std::shared_ptr<Mesh> &sphereMesh = std::make_shared<Mesh>(data, indices);
    sphereMesh->SetName("Sphere");

    AABB sphereAABB(glm::vec3(-radius, -radius, -radius), glm::vec3(radius, radius, radius));
    sphereMesh->SetAABB(sphereAABB);

    return sphereMesh;
}