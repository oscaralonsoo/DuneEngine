#include "ModuleRenderer.h"
#include "RendererAPI.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "Renderer.h"
#include "ModuleCamera.h"
#include "ModuleWindow.h"
#include "MeshComponent.h"
#include "TransformComponent.h"

ModuleRenderer::ModuleRenderer() : Module()
{
    name = "render";
}

bool ModuleRenderer::Start()
{
    shader = new Shader("C:/Users/alons/Documents/GitHub/DuneEngine/Assets/shaders/Shader.glsl");
    shader->Bind();
    shader->SetInt("texture1", 0);
    shader->SetInt("texture2", 1);
    texture1 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");
    texture2 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");
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
    // cameraData.view = glm::inverse(target->GetCameraTransform());
    // cameraData.projection = target->GetCamera().GetProjection();
    // cameraData.position = target->GetCameraTransform()[3];

    // FIXME: Modulate in functions or switch
    for (GameObject *go : Engine::GetInstance().scene.get()->GetGameObjects())
    {
        MeshComponent *meshComp = go->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        TransformComponent *transformComp = go->GetComponent<TransformComponent>();

        RenderObject renderObject;
        // rcmd.transform = transformComp->GetTransformMatrix();
        renderObject.mesh = meshComp->GetMesh();

        Renderer::Submit(renderObject);
    }

    Renderer::ForwardPass(/*target*/);
    // ----------------------
    shader->Bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)SDL_GetTicks() / 1000.0f,
                        glm::vec3(0.5f, 1.0f, 0.0f));

    glm::mat4 view = Engine::GetInstance().camera.get()->camera.GetViewMatrix();

    int width, height;
    SDL_GetWindowSizeInPixels(Engine::GetInstance().window.get()->GetWindow(), &width, &height);
    float aspect = (height > 0) ? (float)width / (float)height : 4.0f / 3.0f;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    shader->SetMat4("model", model);
    shader->SetMat4("view", view);
    shader->SetMat4("projection", projection);

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
