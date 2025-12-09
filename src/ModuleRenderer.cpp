#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Renderer.h"
#include "ModuleWindow.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include <functional>

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

    RendererAPI::SetClearColor({0.03f, 0.03f, 0.03f, 1.0f});
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Enviar todos los objetos a las colas de render
    std::function<void(std::shared_ptr<GameObject>)> renderGameObject = [&](std::shared_ptr<GameObject> go) {
        MaterialComponent* materialComp = go->GetComponent<MaterialComponent>();
        MeshComponent* meshComp = go->GetComponent<MeshComponent>();
        TransformComponent* transformComp = go->GetComponent<TransformComponent>();
        if (materialComp && meshComp && transformComp) {
            RenderObject ro;
            ro.transform = transformComp->GetWorldTransform();
            ro.mesh = meshComp->GetMesh();
            ro.material = materialComp->GetMaterial();
            ro.selected = go->IsSelected();

            Renderer::Submit(ro);
        }

        // Render children recursively
        for (auto child : go->GetChildren()) {
            renderGameObject(child);
        }
    };

    for (auto go : Engine::GetInstance().scene->GetGameObjects())
    {
        renderGameObject(go);
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
    delete renderCamera;

    return true;
}
