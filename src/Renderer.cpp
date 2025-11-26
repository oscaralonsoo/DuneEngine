#include "Renderer.h"
#include "ModuleRenderer.h"
#include "EditorCamera.h"
#include "Engine.h"
#include "RendererAPI.h"
#include "PrimitiveMesh.h"
#include "Globals.h"

std::vector<RenderObject> Renderer::sOpaqueRenderQueue;
std::vector<RenderObject> Renderer::sTransparentRenderQueue;
std::shared_ptr<Shader> Renderer::sSkyboxShader;
std::shared_ptr<Cubemap> Renderer::sCubemap;
std::shared_ptr<Mesh> Renderer::sSkyboxCube;

void Renderer::Init()
{
    ilInit();

    std::vector<std::shared_ptr<Texture>> faces;
    faces.push_back(std::make_shared<Texture>("Assets/skybox/right.jpg", true, true));
    faces.push_back(std::make_shared<Texture>("Assets/skybox/left.jpg", true, true));
    faces.push_back(std::make_shared<Texture>("Assets/skybox/top.jpg", true, true));
    faces.push_back(std::make_shared<Texture>("Assets/skybox/bottom.jpg", true, true));
    faces.push_back(std::make_shared<Texture>("Assets/skybox/front.jpg", true, true));
    faces.push_back(std::make_shared<Texture>("Assets/skybox/back.jpg", true, true));

    sCubemap = std::make_shared<Cubemap>(faces);
    sSkyboxShader = std::make_shared<Shader>("assets/shaders/SkyboxShader.glsl");
    sSkyboxCube = PrimitiveMesh::CreateCube();
}

void Renderer::ForwardPass()
{
    // Sort by material and mesh to minimize state changes
    std::sort(sOpaqueRenderQueue.begin(), sOpaqueRenderQueue.end(), [](const RenderObject &a, const RenderObject &b)
              { return std::tie(a.material, a.mesh) < std::tie(b.material, b.mesh); });

    for (const auto &renderObject : sOpaqueRenderQueue)
    {
        Material *material = renderObject.material.get();
        material->Use();

        const std::shared_ptr<Shader> &shader = material->GetShader();

        shader->Bind();

        Mesh *mesh = renderObject.mesh.get();

        if (!mesh)
            continue;

        RendererAPI::DrawIndexed(mesh->GetVertexArray());
    }

    ResetRenderState();

    sOpaqueRenderQueue.clear();
}

void Renderer::TransparentPass()
{
    glm::vec3 cameraPos = Engine::GetInstance().renderer.get()->renderCamera->GetPosition();

    std::sort(sTransparentRenderQueue.begin(), sTransparentRenderQueue.end(), [&cameraPos](const RenderObject &a, const RenderObject &b)
              {
        float distA = glm::length(cameraPos - glm::vec3(a.transform[3]));
        float distB = glm::length(cameraPos - glm::vec3(b.transform[3]));
        return distA > distB; });

    RendererAPI::SetDepthMask(false);

    for (const auto &renderObject : sTransparentRenderQueue)
    {
        Material *material = renderObject.material.get();
        if (material == nullptr)
        {
            // material = s_RendererData.DefaultMaterial.get();
        }

        material->Use();

        const std::shared_ptr<Shader> &shader = material->GetShader();

        shader->Bind();

        Mesh *mesh = renderObject.mesh.get();

        if (mesh == nullptr)
        {
            // mesh = s_RendererData.MissingMesh.get();
        }

        const MaterialRenderSettings &settings = material->GetRenderSettings();
        switch (settings.cullMode)
        {
        case MaterialRenderSettings::CullMode::Front:
            RendererAPI::SetCullFace(CullFace::Front);
            break;
        case MaterialRenderSettings::CullMode::Back:
            RendererAPI::SetCullFace(CullFace::Back);
            break;
        case MaterialRenderSettings::CullMode::None:
            RendererAPI::SetFaceCulling(false);
            break;
        }

        if (settings.wireframe)
        {
            RendererAPI::SetPolygonMode(PolygonMode::Line);
        }
        else
        {
            RendererAPI::SetPolygonMode(PolygonMode::Fill);
        }

        RendererAPI::DrawIndexed(mesh->GetVertexArray());
    }

    ResetRenderState();

    sTransparentRenderQueue.clear();
}

void Renderer::SkyboxPass()
{
    RendererAPI::ClearDepth();
    RendererAPI::SetDepthMask(false);
    RendererAPI::SetDepthFunc(DepthFunc::LEQUAL);

    // Bind del shader
    sSkyboxShader->Bind();

    // Bind del cubemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sCubemap->GetID());
    sSkyboxShader->SetInt("skybox", 0);

    // Matrices: view sin traslación, projection normal
    auto camera = Engine::GetInstance().renderer->renderCamera;
    glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix())); // elimina la traslación
    glm::mat4 proj = camera->GetProjectionMatrix();

    sSkyboxShader->SetMat4("view", view);
    sSkyboxShader->SetMat4("projection", proj);

    RendererAPI::SetCullFace(CullFace::Front);

    // Dibujar el cubo del skybox
    RendererAPI::DrawIndexed(sSkyboxCube->GetVertexArray());
    RendererAPI::SetDepthMask(true);

    ResetRenderState();
}

void Renderer::Submit(const RenderObject &renderObject)
{
    if (!renderObject.material)
    {
        sOpaqueRenderQueue.push_back(renderObject);
        return;
    }

    const auto &settings = renderObject.material->GetRenderSettings();

    if (settings.transparencyMode == MaterialRenderSettings::TransparencyMode::Disabled)
    {
        sOpaqueRenderQueue.push_back(renderObject);
    }
    else
    {
        sTransparentRenderQueue.push_back(renderObject);
    }
}

void Renderer::ResetRenderState()
{
    RendererAPI::SetCullFace(CullFace::Back);
    RendererAPI::SetFaceCulling(true);
    RendererAPI::SetDepthMask(true);
    RendererAPI::SetDepthFunc(DepthFunc::LESS);
    RendererAPI::SetPolygonMode(PolygonMode::Fill);
}