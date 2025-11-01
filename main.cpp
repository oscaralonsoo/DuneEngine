#define IL_NO_WCHAR_T
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include "shader_s.h"
#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <vector>
#include <string>

static glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
static glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
static bool  gFirstMouse = true;
static float gYaw   = -90.0f;
static float gPitch =  0.0f;
static float gFov   =  45.0f;
static float gLastX =  400.0f;
static float gLastY =  300.0f;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static float kCubeVertices[] = {
    // back face
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f,
     0.5f,-0.5f,-0.5f, 1.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f,
    // front face
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f,
    // left face
    -0.5f, 0.5f, 0.5f, 1.0f,0.0f,
    -0.5f, 0.5f,-0.5f, 1.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,0.0f,
    // right face
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,
     0.5f,-0.5f,-0.5f, 0.0f,1.0f,
     0.5f,-0.5f,-0.5f, 0.0f,1.0f,
     0.5f,-0.5f, 0.5f, 0.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,
    // bottom face
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,
     0.5f,-0.5f,-0.5f, 1.0f,1.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,
    // top face
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,0.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f
};

static GLuint LoadTextureDevIL(const char* path, bool genMipmaps = true) {
    // 1) Leer el archivo en memoria
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        SDL_Log("No se pudo abrir %s", path);
        return 0;
    }
    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>());

    // 2) Crear imagen y cargar desde memoria con DevIL (independiente de wchar/char)
    ILuint img = 0;
    ilGenImages(1, &img);
    ilBindImage(img);

    if (!ilLoadL(IL_TYPE_UNKNOWN, buf.data(), (ILuint)buf.size())) {
        ILenum err = ilGetError();
        SDL_Log("DevIL: fallo al cargar '%s' desde memoria (err=%d)", path, (int)err);
        ilDeleteImages(1, &img);
        return 0;
    }

    // 3) Normalizar a RGBA8 y subir a OpenGL
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    const int w = ilGetInteger(IL_IMAGE_WIDTH);
    const int h = ilGetInteger(IL_IMAGE_HEIGHT);
    const void* pixels = ilGetData();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (genMipmaps) glGenerateMipmap(GL_TEXTURE_2D);

    ilDeleteImages(1, &img);
    return tex;
}


int main(int argc, char *args[])
{
    // --- Initialize SDL ---
    // Initialize SDL video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        return -1;
    }

    // --- Set OpenGL Attributes ---
    // Configure OpenGL context properties
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // --- Create Window ---
    // Create SDL window with OpenGL support
    SDL_Window *window = SDL_CreateWindow("DuneEngine", 800, 600, SDL_WINDOW_OPENGL);
    if (!window)
    {
        SDL_Quit();
        return -1;
    }

    // --- Create OpenGL Context ---
    // Create a rendering context for the window
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowRelativeMouseMode(window, true);

    // --- Load OpenGL Functions ---
    // Initialize GLAD to load OpenGL function pointers
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
    {
        SDL_Log("Failed to initialize GLAD\n");
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    // --- Set Viewport ---
    // Define the OpenGL rendering area
    glViewport(0, 0, 800, 600);

    // --- Log GPU Information ---
    // Print graphics vendor, renderer, and version info
    SDL_Log("Vendor: %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("OpenGL version supported %s", glGetString(GL_VERSION));
    SDL_Log("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    SDL_Log("OpenGL version (from GLAD): %d.%d", GLVersion.major, GLVersion.minor);
    SDL_Log("OpenGL profile (from GLAD): %s", GLAD_GL_VERSION_3_1 ? "Core 3.1+" : "Other");

    Shader shader("7.3.camera.vs", "7.3.camera.fs");

    GLuint VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);

    // location 0 -> vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // location 1 -> vec2 uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint texture1 = LoadTextureDevIL("resources/textures/container.jpg");
    if (!texture1) SDL_Log("No se pudo cargar container.jpg");

    GLuint texture2 = LoadTextureDevIL("resources/textures/awesomeface.png");
    if (!texture2) SDL_Log("No se pudo cargar awesomeface.png");

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    glm::vec3 cubePositions[] = {
        { 0.0f,  0.0f,  0.0f},
        { 2.0f,  5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        { 2.4f, -0.4f, -3.5f},
        {-1.7f,  3.0f, -7.5f},
        { 1.3f, -2.0f, -2.5f},
        { 1.5f,  2.0f, -2.5f},
        { 1.5f,  0.2f, -1.5f},
        {-1.3f,  1.0f, -1.5f}
    };

    bool running = true;

    lastFrame = SDL_GetTicks() / 1000.0f;

    while (running)
    {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                {
                    running = false;
                }
                else if (event.key.key == SDLK_1)
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
                }
                else if (event.key.key == SDLK_2)
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Fill mode
                }
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                int w, h;
                SDL_GetWindowSizeInPixels(window, &w, &h);
                glViewport(0, 0, w, h);
                break;
                case SDL_EVENT_MOUSE_MOTION: {
                    // deltas en modo relativo
                    float xoffset = (float)event.motion.xrel;
                    float yoffset = (float)event.motion.yrel; // y invertida manualmente
                    const float sensitivity = 0.1f;
                    xoffset *= sensitivity;
                    yoffset *= sensitivity;

                    gYaw   += xoffset;
                    gPitch -= yoffset;        // invertimos aquí

                    if (gPitch > 89.0f)  gPitch = 89.0f;
                    if (gPitch < -89.0f) gPitch = -89.0f;

                    glm::vec3 front;
                    front.x = cos(glm::radians(gYaw)) * cos(glm::radians(gPitch));
                    front.y = sin(glm::radians(gPitch));
                    front.z = sin(glm::radians(gYaw)) * cos(glm::radians(gPitch));
                    cameraFront = glm::normalize(front);
                } break;

                case SDL_EVENT_MOUSE_WHEEL: {
                    gFov -= (float)event.wheel.y;  // rueda hacia arriba -> acercar
                    if (gFov < 1.0f)  gFov = 1.0f;
                    if (gFov > 45.0f) gFov = 45.0f;
                } break;

            }
        }

        const bool* ks = SDL_GetKeyboardState(NULL);
        float speed = 2.5f * deltaTime;

        if (ks[SDL_SCANCODE_W]) cameraPos += speed * cameraFront;
        if (ks[SDL_SCANCODE_S]) cameraPos -= speed * cameraFront;
        if (ks[SDL_SCANCODE_A]) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (ks[SDL_SCANCODE_D]) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        shader.use();

        // cachea locations
        GLint uModel = glGetUniformLocation(shader.ID, "model");
        GLint uView  = glGetUniformLocation(shader.ID, "view");
        GLint uProj  = glGetUniformLocation(shader.ID, "projection");

        float t = SDL_GetTicks() / 1000.0f;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        float aspect = (height > 0) ? (float)width / (float)height : 4.0f/3.0f;

        // antes: 45.0f fijo
        // después: gFov (modificable con la rueda)
        glm::mat4 projection = glm::perspective(glm::radians(gFov), aspect, 0.1f, 100.0f);

        // sube a los uniforms (como ya haces)
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i; // grados
            model = glm::rotate(model, glm::radians(angle) + t, glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // INTERCAMBIAR buffers (mostrar en pantalla)
        SDL_GL_SwapWindow(window);

    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);

    // Cleanup
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
