#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
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
    shader = new Shader("C:/Users/alons/Documents/GitHub/DuneEngine/Assets/shaders/Shader.glsl");
    shader->Bind();
    shader->SetInt("texture1", 0);
    shader->SetInt("texture2", 1);
    texture1 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");
    texture2 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");
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

    // FIXME: Modulate in functions or switch
    for (GameObject *go : Engine::GetInstance().scene.get()->GetGameObjects())
    {
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        glm::mat4 model = glm::mat4(1.0f);
        shader->SetMat4("model", model);

        TransformComponent *transformComp = go->GetComponent<TransformComponent>();

        RenderObject renderObject;
        // rcmd.transform = transformComp->GetTransformMatrix();
        renderObject.mesh = meshComp->GetMesh();

        Renderer::Submit(renderObject);
    }

    Renderer::ForwardPass(/*target*/);

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
