#include "Engine.h"
#include "Renderer.h"
#include "ModuleRenderer.h"
#include "ModuleResource.h"
#include "EditorCamera.h"
#include "Engine.h"
#include "RendererAPI.h"
#include "PrimitiveMesh.h"
#include "Globals.h"

std::vector<RenderObject> Renderer::sOpaqueRenderQueue;
std::vector<RenderObject> Renderer::sTransparentRenderQueue;
std::vector<RenderObject> Renderer::sSelectedRenderQueue;
std::shared_ptr<Shader> Renderer::sSkyboxShader;
std::shared_ptr<Shader> Renderer::sSingleColorShader;
std::shared_ptr<Cubemap> Renderer::sCubemap;
std::shared_ptr<Mesh> Renderer::sSkyboxCube;

void Renderer::Init()
{
    // TEMPORARY: DevIL origin is changed here to correctly load skybox textures.
    // This is a workaround and should be replaced in the future with a proper
    // texture import/orientation handling system.
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    std::vector<std::shared_ptr<Texture>> faces;
    std::shared_ptr<ModuleResource> resourceManager = Engine::GetInstance().resourceManager;
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/right.jpg")));
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/left.jpg")));
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/top.jpg")));
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/bottom.jpg")));
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/front.jpg")));
    faces.push_back(std::dynamic_pointer_cast<Texture>(resourceManager->RequestResource("Assets/skybox/back.jpg")));

    // TEMPORARY: Restore DevIL origin to default.
    // This should be removed once texture orientation is handled properly.
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    sCubemap = std::make_shared<Cubemap>(faces);
    sSkyboxShader = std::make_shared<Shader>("assets/shaders/SkyboxShader.glsl");
    sSkyboxCube = PrimitiveMesh::CreateCube();
    sSingleColorShader = std::make_shared<Shader>("assets/shaders/SingleColorShader.glsl");
}

void Renderer::ForwardPass()
{
    std::sort(sOpaqueRenderQueue.begin(), sOpaqueRenderQueue.end(), [](const RenderObject &a, const RenderObject &b)
              { return std::tie(a.material, a.mesh) < std::tie(b.material, b.mesh); });

    auto camera = Engine::GetInstance().renderer->renderCamera;

    for (const auto &renderObject : sOpaqueRenderQueue)
    {
        Material *material = renderObject.material.get();
        if (!material)
            continue;
        material->Use();
        auto shader = material->GetShader();
        shader->Bind();
        shader->SetMat4("view", camera->GetViewMatrix());
        shader->SetMat4("projection", camera->GetProjectionMatrix());
        shader->SetMat4("model", renderObject.transform);

        // Escribir stencil solo si el objeto está seleccionado
        if (renderObject.selected)
        {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            glStencilMask(0x00);
        }

        RendererAPI::DrawIndexed(renderObject.mesh->GetVertexArray());
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
        auto camera = Engine::GetInstance().renderer->renderCamera;
        shader->SetMat4("view", camera->GetViewMatrix());
        shader->SetMat4("projection", camera->GetProjectionMatrix());
        shader->SetMat4("model", renderObject.transform);

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
    RendererAPI::SetDepthFunc(DepthFunc::Lequal);

    sSkyboxShader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sCubemap->GetID());

    auto camera = Engine::GetInstance().renderer->renderCamera;
    glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix()));
    glm::mat4 proj = camera->GetProjectionMatrix();

    sSkyboxShader->SetMat4("view", view);
    sSkyboxShader->SetMat4("projection", proj);

    RendererAPI::SetCullFace(CullFace::Front);

    RendererAPI::DrawIndexed(sSkyboxCube->GetVertexArray());
    RendererAPI::SetDepthMask(true);

    ResetRenderState();
}
void Renderer::SelectedPass()
{
    if (sSelectedRenderQueue.empty())
        return;

    auto camera = Engine::GetInstance().renderer->renderCamera;

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    sSingleColorShader->Bind();
    sSingleColorShader->SetMat4("view", camera->GetViewMatrix());
    sSingleColorShader->SetMat4("projection", camera->GetProjectionMatrix());
    sSingleColorShader->SetVec3("outlineColor", glm::vec3(0.87f, 0.72f, 0.53f));
    sSingleColorShader->SetFloat("outlineThickness", 0.07f);

    for (auto &renderObject : sSelectedRenderQueue)
    {
        glm::mat4 scaled = renderObject.transform * glm::scale(glm::mat4(1.0f), glm::vec3(1.05f));
        sSingleColorShader->SetMat4("model", scaled);

        glCullFace(GL_FRONT);
        RendererAPI::DrawIndexed(renderObject.mesh->GetVertexArray());
    }

    glEnable(GL_DEPTH_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    ResetRenderState();

    sSelectedRenderQueue.clear();
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

    if (renderObject.selected)
    {
        sSelectedRenderQueue.push_back(renderObject);
    }
}

void Renderer::ResetRenderState()
{
    RendererAPI::SetCullFace(CullFace::Back);
    RendererAPI::SetFaceCulling(true);
    RendererAPI::SetDepthMask(true);
    RendererAPI::SetDepthFunc(DepthFunc::Less);
    RendererAPI::SetPolygonMode(PolygonMode::Fill);
}