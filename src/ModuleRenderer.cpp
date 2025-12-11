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
#include "Globals.h"

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

    void DebugDrawAABB(const AABB& box,
                       const glm::mat4& view,
                       const glm::mat4& projection,
                       const glm::vec3& color)
    {
        // 8 vértices de la caja
        glm::vec3 v[8];
        box.GetVertices(v);

        // 12 aristas -> 24 puntos (líneas)
        glm::vec3 lines[24] = {
            // Cara frontal
            v[0], v[1],
            v[1], v[3],
            v[3], v[2],
            v[2], v[0],
            // Cara trasera
            v[4], v[5],
            v[5], v[7],
            v[7], v[6],
            v[6], v[4],
            // Conectores
            v[0], v[4],
            v[1], v[5],
            v[2], v[6],
            v[3], v[7],
        };

        static GLuint sVAO = 0;
        static GLuint sVBO = 0;

        if (sVAO == 0)
        {
            glGenVertexArrays(1, &sVAO);
            glGenBuffers(1, &sVBO);
        }

        glBindVertexArray(sVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);

        // Atributo posición en location 0 (asumimos layout(location = 0) in vec3 aPosition;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // Usamos el shader de color único del Renderer
        auto singleColorShader = Renderer::GetSingleColorShader(); // lo añadimos ahora
        if (!singleColorShader)
            return;

        singleColorShader->Bind();
        singleColorShader->SetMat4("view",       view);
        singleColorShader->SetMat4("projection", projection);
        singleColorShader->SetMat4("model",      glm::mat4(1.0f));
        singleColorShader->SetVec3("outlineColor", color);
        singleColorShader->SetFloat("outlineThickness", 0.0f); // no lo usamos aquí

        glDrawArrays(GL_LINES, 0, 24);

        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void DebugDrawRay(const Ray& ray,
                      float length,
                      const glm::mat4& view,
                      const glm::mat4& projection,
                      const glm::vec3& color)
    {
        glm::vec3 p0 = ray.origin;
        glm::vec3 p1 = ray.origin + ray.direction * length;

        glm::vec3 line[2] = { p0, p1 };

        static GLuint sRayVAO = 0;
        static GLuint sRayVBO = 0;

        if (sRayVAO == 0)
        {
            glGenVertexArrays(1, &sRayVAO);
            glGenBuffers(1, &sRayVBO);
        }

        glBindVertexArray(sRayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sRayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        auto singleColorShader = Renderer::GetSingleColorShader();
        if (!singleColorShader)
            return;

        singleColorShader->Bind();
        singleColorShader->SetMat4("view",       view);
        singleColorShader->SetMat4("projection", projection);
        singleColorShader->SetMat4("model",      glm::mat4(1.0f));
        singleColorShader->SetVec3("outlineColor", color);
        singleColorShader->SetFloat("outlineThickness", 0.0f);

        glDrawArrays(GL_LINES, 0, 2);

        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
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

        bool inside = mFrustum.ContainsAABB(worldBox);

        if (!inside)
            continue;

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

    if (!isPlaying)
    {
        auto* scene     = Engine::GetInstance().scene.get();
        Raycaster* rc   = scene->GetRaycaster();
        if (rc && rc->HasLastRay())
        {
            Ray lastRay = rc->GetLastRay();
            DebugDrawRay(lastRay, 100.0f, view, projection, glm::vec3(1.0f, 1.0f, 0.0f));
        }
    }

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
