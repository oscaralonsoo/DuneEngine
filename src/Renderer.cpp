#include "Renderer.h"
#include "RendererAPI.h"
#include "Globals.h"

std::vector<RenderObject> Renderer::sOpaqueRenderQueue;

void Renderer::ForwardPass()
{
    RendererAPI::SetClearColor({0.03f, 0.03f, 0.03f, 1.0});
    RendererAPI::Clear();

    // FXIME: sort by material and mesh to minimize state changes (if we had material class)
    //  std::sort(sOpaqueRenderQueue.begin(), sOpaqueRenderQueue.end(), [](const RenderObject &a, const RenderObject &b)
    //            { return std::tie(a.material, a.mesh) < std::tie(b.material, b.mesh); });

    for (const auto &renderObject : sOpaqueRenderQueue)
    {
        Material *material = renderObject.material.get();
        material->Use();

        const std::shared_ptr<Shader> &shader = material->GetShader();

        shader->Bind();

        Mesh *mesh = renderObject.mesh.get();

        if (!mesh)
            continue;

        RendererAPI::SetCullFace(CullFace::Back);
        RendererAPI::SetDepthMask(true);
        RendererAPI::SetPolygonMode(PolygonMode::Fill);

        RendererAPI::DrawIndexed(mesh->GetVertexArray());
    }
    sOpaqueRenderQueue.clear();
}

void Renderer::Submit(const RenderObject &renderObject)
{
    sOpaqueRenderQueue.push_back(renderObject);
}
