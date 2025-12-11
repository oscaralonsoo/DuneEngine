#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Renderer.h"
#include "ModuleWindow.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "AABB.h"
#include "CameraComponent.h"
#include "GameTime.h"

namespace
{
    ICamera* FindGameCamera()
    {
        auto* scene = Engine::GetInstance().scene.get();
        for (auto& go : scene->GetGameObjects())
        {
            if (auto* cam = go->GetComponent<CameraComponent>())
                return cam;
        }
        return nullptr;
    }
}


ModuleRenderer::ModuleRenderer() : Module()
{
    name = "renderer";
}

bool ModuleRenderer::Start()
{
    // Crear cámara de render
    editorCamera = new EditorCamera(
        45.0f,
        16.0f / 9.0f,
        0.1f,
        1000.0f);

    renderCamera = editorCamera;

    RendererAPI::Init();
    Renderer::Init();
    return true;
}

bool ModuleRenderer::PreUpdate()
{
    return true;
}

bool ModuleRenderer::Update()
{
    bool isPlaying = GameTime::IsPlaying();

    // 1) Elegir qué cámara usar este frame
    if (isPlaying)
    {
        // Intentar usar una cámara de juego
        if (ICamera* gameCam = FindGameCamera())
            renderCamera = gameCam;
        else
            renderCamera = editorCamera; // fallback
    }
    else
    {
        renderCamera = editorCamera;
    }

    // 2) Actualizar cámara del editor solo en modo editor
    if (!isPlaying && editorCamera)
    {
        editorCamera->Update();
    }

    // 3) Ajustar viewport de la cámara activa
    int w, h;
    SDL_GetWindowSizeInPixels(
        Engine::GetInstance().window->GetWindow(),
        &w, &h);

    if (renderCamera)
        renderCamera->SetViewportSize((float)w, (float)h);

    // 4) Frustum y clear
    glm::mat4 view       = renderCamera->GetViewMatrix();
    glm::mat4 projection = renderCamera->GetProjectionMatrix();
    mFrustum.Update(view, projection);

    RendererAPI::SetClearColor({0.03f, 0.03f, 0.03f, 1.0f});
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (auto go : Engine::GetInstance().scene->GetGameObjects())
    {
        MaterialComponent*  materialComp  = go->GetComponent<MaterialComponent>();
        MeshComponent*      meshComp      = go->GetComponent<MeshComponent>();
        TransformComponent* transformComp = go->GetComponent<TransformComponent>();
        if (!materialComp || !meshComp || !transformComp) 
            continue;

        const std::shared_ptr<Mesh>& mesh = meshComp->GetMesh();
        if (!mesh)
            continue;

        glm::mat4 world = transformComp->GetWorldTransform();

        const AABB& localBox = mesh->GetAABB();
        AABB worldBox        = TransformAABB(localBox, world);

        if (!mFrustum.ContainsAABB(worldBox))
        {
            continue;
        }

        RenderObject ro;
        ro.transform = world;
        ro.mesh      = mesh;
        ro.material  = materialComp->GetMaterial();
        ro.selected  = go->IsSelected();

        Renderer::Submit(ro);
    }

    Renderer::SkyboxPass();
    Renderer::ForwardPass();
    Renderer::TransparentPass();
    Renderer::SelectedPass();

    return true;
}

bool ModuleRenderer::PostUpdate()
{
    return true;
}

bool ModuleRenderer::CleanUp()
{
    delete editorCamera;
    editorCamera = nullptr;
    renderCamera = nullptr;

    return true;
}
