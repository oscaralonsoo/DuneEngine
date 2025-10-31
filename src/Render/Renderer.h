#pragma once
#include <glad/glad.h>

enum class RenderMode { Solid, WireHidden };

class Renderer {
public:
    void clear(float r, float g, float b, float a);
    void renderMesh(const Mesh& mesh, Shader& shader, RenderMode mode);
};
