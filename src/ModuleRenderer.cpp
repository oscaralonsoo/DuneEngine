#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Renderer.h"
#include "MeshComponent.h"
#include "TransformComponent.h"
#include <glm

ModuleRenderer::ModuleRenderer() : Module()
{
    name = "render";
}

bool ModuleRenderer::Start()
{
    RendererAPI::Init();
    return true;
}

bool ModuleRenderer::PreUpdate()
{
    return true;
}

bool ModuleRenderer::Update()
{
    CameraData cameraData;
    cameraData.view = glm::inverse(target->GetCameraTransform());
    cameraData.projection = target->GetCamera().GetProjection();
    cameraData.position = target->GetCameraTransform()[3];

    // FIXME: Modulate in functions or switch
    for (GameObject *go : Engine::GetInstance().scene.get()->GetGameObjects())
    {
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        TransformComponent *transformComp = go->GetComponent<TransformComponent>();

        RenderCommand rcmd;
        // rcmd.transform = transformComp->GetTransformMatrix();
        rcmd.mesh = meshComp->GetMesh();

        Renderer::Submit(rcmd);
    }

    Renderer::Render();

    return true;
}

bool ModuleRenderer::PostUpdate()
{
    return true;
}

bool ModuleRenderer::CleanUp()
{
    return true;
}
