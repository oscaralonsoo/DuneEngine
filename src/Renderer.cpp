#include "Renderer.h"
#include "RendererAPI.h"

std::vector<RenderCommand> Renderer::sRenderQueue;

void Renderer::Render()
{
    RendererAPI::SetClearColor({0.03f, 0.03f, 0.03f, 1.0f});
    RendererAPI::Clear();

    for (const auto &command : sRenderQueue)
    {
        Mesh *mesh = command.mesh.get();
        if (!mesh)
            continue;

        RendererAPI::SetCullFace(CullFace::Back);
        RendererAPI::SetFaceCulling(true);
        RendererAPI::SetDepthMask(true);
        RendererAPI::SetPolygonMode(PolygonMode::Fill);

        RendererAPI::DrawIndexed(mesh->GetVertexArray());
    }
}

void Renderer::Submit(const RenderCommand &command)
{
    sRenderQueue.push_back(command);
}
