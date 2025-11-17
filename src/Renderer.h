#pragma once

#include "Mesh.h"

struct RenderCommand
{
    glm::mat4 transform = glm::mat4(1.0f);
    std::shared_ptr<Mesh> mesh;
};

class Renderer
{
public:
    static void Init();
    static void Render();

    static void Submit(const RenderCommand& command);

    private:
    static std::vector<RenderCommand> sRenderQueue;
};