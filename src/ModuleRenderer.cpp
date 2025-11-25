#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "Texture.h"
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
    // Inicializar la textura con shared_ptr
    texture = std::make_shared<Texture>("Assets/textures/basic.jpg");

    // Crear cámara de render
    renderCamera = new EditorCamera(
        45.0f,
        16.0f / 9.0f,
        0.1f,
        1000.0f);

    RendererAPI::Init();
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

        shader = materialComp->GetMaterial()->GetShader();
        shader->Bind();
        shader->SetMat4("view", renderCamera->GetViewMatrix());
        shader->SetMat4("projection", renderCamera->GetProjectionMatrix());
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        TransformComponent *transformComp = go->GetComponent<TransformComponent>();
        shader->SetMat4("model", transformComp->GetWorldTransform());

        RenderObject renderObject;
        renderObject.mesh = meshComp->GetMesh();
        renderObject.material = materialComp->GetMaterial();

        Renderer::Submit(renderObject);
    }

    Renderer::ForwardPass();
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
