#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Renderer.h"
#include "ModuleWindow.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"

ModuleRenderer::ModuleRenderer() : Module()
{
    name = "renderer";
}

bool ModuleRenderer::Start()
{
    // Crear cámara de render
    renderCamera = new EditorCamera(
        45.0f,
        16.0f / 9.0f,
        0.1f,
        1000.0f);

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
    renderCamera->Update();

    int w, h;
    SDL_GetWindowSizeInPixels(
        Engine::GetInstance().window->GetWindow(),
        &w, &h);

    renderCamera->SetViewportSize(w, h);

    // Renderizar objetos de la escena
    for (std::shared_ptr<GameObject> go : Engine::GetInstance().scene.get()->GetGameObjects())
    {

        MaterialComponent *materialComp = go->GetComponent<MaterialComponent>();
        if (!materialComp || !materialComp->GetMaterial())
            continue;

        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        TransformComponent *transformComp = go->GetComponent<TransformComponent>();

        RenderObject renderObject;
        renderObject.transform = transformComp->GetWorldTransform();
        renderObject.mesh = meshComp->GetMesh();
        renderObject.material = materialComp->GetMaterial();
        renderObject.transform = transformComp->GetWorldTransform();
        renderObject.selected = go->IsSelected();

        Renderer::Submit(renderObject);
    }
    RendererAPI::SetClearColor({0.03f, 0.03f, 0.03f, 1.0});
    RendererAPI::Clear();
    
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
    delete renderCamera;

    return true;
}
