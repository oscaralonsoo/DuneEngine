#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "Texture.h"
#include "ModuleScene.h"
#include "Renderer.h"
#include "ModuleWindow.h"
#include "MeshComponent.h"
#include "TransformComponent.h"

ModuleRenderer::ModuleRenderer() : Module()
{
    name = "renderer";
}

bool ModuleRenderer::Start()
{
    shader = new Shader("Assets/shaders/Shader.glsl");

    // Inicializar la textura con shared_ptr
    texture = std::make_shared<Texture>("Assets/textures/basic.jpg");

    // Setear uniform de textura en la unidad 0
    shader->Bind();
    shader->SetInt("texture", 0);
    shader->Unbind();

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

    shader->Bind();
    shader->SetMat4("view", renderCamera->GetViewMatrix());
    shader->SetMat4("projection", renderCamera->GetProjectionMatrix());

    // Renderizar objetos de la escena
    for (GameObject *go : Engine::GetInstance().scene.get()->GetGameObjects())
    {
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        glm::mat4 model = glm::mat4(1.0f);
        TransformComponent *transformComp = go->GetComponent<TransformComponent>();
        // if (transformComp)
        //     model = transformComp->GetTransformMatrix();

        shader->SetMat4("model", model);

        RenderObject renderObject;
        renderObject.mesh = meshComp->GetMesh();

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
    delete shader;

    return true;
}
