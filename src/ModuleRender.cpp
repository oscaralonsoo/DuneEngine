#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "ModuleCamera.h"
#include "ModuleTextures.h"
#include "Engine.h"

static float kCubeVertices[] = {
    // back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    // front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    // left face
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    // right face
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    // bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    // top face
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

ModuleRender::ModuleRender() : Module()
{
	name = "render";
}

ModuleRender::~ModuleRender()
{}

bool ModuleRender::Start()
{
    shader = new Shader("C:/Users/alons/Documents/GitHub/DuneEngine/Assets/shaders/Shader.vs", "C:/Users/alons/Documents/GitHub/DuneEngine/Assets/shaders/Shader.fs");


    // --- Setup cube VAO/VBO ---
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    shader->use();
    shader->setInt("texture1", 0);
    shader->setInt("texture2", 1);
    
    texture1 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");
    texture2 = Engine::GetInstance().textures.get()->LoadTexture("Assets/textures/basic.jpg");

    if (texture1 == 0 || texture2 == 0)
    {
        SDL_Log("Failed to load textures.");
        return false;
    }
	return true;
}

// Called each loop iteration
bool ModuleRender::PreUpdate()
{
	return true;
}

bool ModuleRender::Update()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    shader->use();

    // MODEL: rotación continua (eje 0.5,1,0 como en 6.2)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)SDL_GetTicks() / 1000.0f,
                        glm::vec3(0.5f, 1.0f, 0.0f));

    // VIEW: alejar cámara en -Z
    glm::mat4 view = Engine::GetInstance().camera.get()->camera.GetViewMatrix();
    
    // PROJECTION: perspectiva
    int width, height;
    SDL_GetWindowSizeInPixels(Engine::GetInstance().window.get()->GetWindow(), &width, &height);
    float aspect = (height > 0) ? (float)width / (float)height : 4.0f / 3.0f;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    // Subir uniforms al shader
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	return true;
}

bool ModuleRender::PostUpdate()
{
    // Clear the color and depth buffers
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // Use shader program
    shader->use();

    // Draw cube
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SDL_GL_SwapWindow(Engine::GetInstance().window.get()->GetWindow());
	return true;
}

// Called before quitting
bool ModuleRender::CleanUp()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);

	return true;
}

// void ModuleRender::SetBackgroundColor(SDL_Color color)
// {
// 	background = color;
// }

// void ModuleRender::SetViewPort(const SDL_Rect& rect)
// {
// 	SDL_ModuleRenderSetViewport(ModuleRenderer, &rect);
// }

// void ModuleRender::ResetViewPort()
// {
// 	SDL_ModuleRenderSetViewport(ModuleRenderer, &viewport);
// }
