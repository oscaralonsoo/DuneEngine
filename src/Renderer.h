#pragma once
#include "Mesh.h"

struct RenderObject
{
    glm::mat4 transform = glm::mat4(1.0f);
    std::shared_ptr<Mesh> mesh;
};

class Renderer
{
public:
    static void Init();

    static void ForwardPass(/*const std::shared_ptr<RenderTarget>& target*/);

    static void Submit(const RenderObject& renderObject);

    private:
    static std::vector<RenderObject> sOpaqueRenderQueue;
    static std::vector<RenderObject> sTransparentRenderQueue;
};