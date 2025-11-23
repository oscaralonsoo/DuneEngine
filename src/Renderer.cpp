#include "Renderer.h"
#include "RendererAPI.h"

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
        // Material *material = command.material.get();

        // if (material == nullptr or material->GetShader() == nullptr)
        // {
        //     material = s_RendererData.DefaultMaterial.get();
        // }

        // material->Use();

        // const Ref<Shader> &shader = material->GetShader();

        // shader->Bind();

        // // Set the irradiance map, is 6 because the first 6 slots are used by the material
        // shader->setInt("irradianceMap", 6);
        // shader->setInt("prefilterMap", 7);
        // shader->setInt("brdfLUT", 8);
        // shader->setMat4("model", command.transform);
        // shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(command.transform))));

        // shader->setBool("showNormals", s_RenderSettings.showNormals);

        Mesh *mesh = renderObject.mesh.get();

        if (!mesh)
            continue;

        RendererAPI::SetCullFace(CullFace::Back);
        RendererAPI::SetDepthMask(true);
        RendererAPI::SetPolygonMode(PolygonMode::Line);

        RendererAPI::DrawIndexed(mesh->GetVertexArray());
    }
}

void Renderer::Submit(const RenderObject &renderObject)
{
    sOpaqueRenderQueue.push_back(renderObject);
}
