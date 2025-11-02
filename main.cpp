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
#include "camera.h" 
#include "model.h"
#include <cfloat> 
#include <cctype>

struct AABB { glm::vec3 min, max; };

static glm::vec3 gOrbitTarget = glm::vec3(0.0f);
static bool      gHasSelection = false;
static bool      gOrbiting = false;        // ALT + RMB
static float     gOrbitDistance = 5.0f;    // se ajusta al seleccionar

static Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
bool rightMouseHeld = false;
bool altHeld = false;

struct SceneItem {
    std::unique_ptr<Model> model;
    glm::mat4              M = glm::mat4(1.0f);
    GLuint                 overrideTex = 0;
};

static std::vector<SceneItem> gScene;
static int gSelectedIndex = -1;

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

static std::unique_ptr<Model> CreateCubeModel(float size = 1.0f, GLuint initialTex = 0)
{
    // kCubeVertices tiene 36 vértices (6 caras * 2 tri * 3 vértices) con (pos.xyz, uv)
    // Normales por cara (back, front, left, right, bottom, top) en el mismo orden del array:
    const glm::vec3 faceNormals[6] = {
        { 0,  0, -1}, // back
        { 0,  0,  1}, // front
        {-1,  0,  0}, // left
        { 1,  0,  0}, // right
        { 0, -1,  0}, // bottom
        { 0,  1,  0}  // top
    };

    std::vector<Vertex> verts;
    verts.reserve(36);

    for (int face = 0; face < 6; ++face) {
        glm::vec3 n = faceNormals[face];
        for (int i = 0; i < 6; ++i) { // 6 vértices por cara
            int base = (face * 6 + i) * 5;
            Vertex v{};
            // posición
            v.Position = glm::vec3(
                kCubeVertices[base + 0] * size,
                kCubeVertices[base + 1] * size,
                kCubeVertices[base + 2] * size
            );
            // normal (plana por cara)
            v.Normal = n;
            // uv
            v.TexCoords = glm::vec2(
                kCubeVertices[base + 3],
                kCubeVertices[base + 4]
            );
            // tangente/bitangente “dummy” (opcional)
            v.Tangent   = glm::vec3(1,0,0);
            v.Bitangent = glm::vec3(0,1,0);
            // huesos a cero
            std::fill(std::begin(v.m_BoneIDs), std::end(v.m_BoneIDs), 0);
            std::fill(std::begin(v.m_Weights), std::end(v.m_Weights), 0.0f);

            verts.push_back(v);
        }
    }

    // Índices 0..35 (coincide con el orden del array)
    std::vector<unsigned int> idx(36);
    for (unsigned int i = 0; i < 36; ++i) idx[i] = i;

    // Textura opcional inicial (como texture_diffuse1)
    std::vector<Texture> tex;
    if (initialTex != 0) {
        Texture t{};
        t.id   = initialTex;
        t.type = "texture_diffuse";
        t.path = "__cube_init__";
        tex.push_back(t);
    }

    // Usa el nuevo ctor de Model (desde memoria)
    auto m = std::make_unique<Model>(verts, idx, tex, /*gamma=*/false);

    // Además, para que funcione el override por drag&drop igual que la casa:
    if (initialTex != 0) m->SetOverrideTexture(initialTex);

    return m;
}

// Convierte (mouseX, mouseY) en un rayo en espacio mundial
static void BuildRayFromScreen(int mouseX, int mouseY, int width, int height,
                               const glm::mat4& view, const glm::mat4& proj,
                               glm::vec3& outOrigin, glm::vec3& outDir)
{
    // NDC en OpenGL: z en [-1, 1]
    float x = (2.0f * mouseX) / (float)width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / (float)height;

    glm::mat4 invVP = glm::inverse(proj * view);

    glm::vec4 pNear = invVP * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 pFar  = invVP * glm::vec4(x, y,  1.0f, 1.0f);
    pNear /= pNear.w;
    pFar  /= pFar.w;

    outOrigin = glm::vec3(pNear);
    outDir    = glm::normalize(glm::vec3(pFar - pNear));
}

// Intersección rayo–AABB (slab method). Devuelve true si hay intersección
static bool RayIntersectsAABB(const glm::vec3& ro, const glm::vec3& rd, const AABB& box, float* tHit = nullptr)
{
    glm::vec3 t1 = (box.min - ro) / rd;
    glm::vec3 t2 = (box.max - ro) / rd;
    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);
    float tEnter = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tExit  = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
    if (tExit >= glm::max(tEnter, 0.0f)) { if (tHit) *tHit = tEnter; return true; }
    return false;
}

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

static AABB ComputeModelAABB(const Model& m, const glm::mat4& modelMatrix)
{
    AABB box;
    box.min = glm::vec3( FLT_MAX);
    box.max = glm::vec3(-FLT_MAX);

    for (const auto& mesh : m.meshes) {
        for (const auto& v : mesh.vertices) {
            glm::vec4 wp = modelMatrix * glm::vec4(v.Position, 1.0f);
            box.min = glm::min(box.min, glm::vec3(wp));
            box.max = glm::max(box.max, glm::vec3(wp));
        }
    }
    return box;
}

static bool HasExt(const std::string& p, const char* ext) {
    auto i = p.find_last_of('.');
    if (i == std::string::npos) return false;
    std::string e = p.substr(i+1);
    for (auto& c : e) c = (char)tolower((unsigned char)c);
    std::string want = ext;
    for (auto& c : want) c = (char)tolower((unsigned char)c);
    return e == want;
}

static bool IsModelPath(const std::string& p) {
    return HasExt(p,"fbx") || HasExt(p,"obj") || HasExt(p,"dae") ||
           HasExt(p,"gltf")|| HasExt(p,"glb") || HasExt(p,"3ds") ||
           HasExt(p,"ply")  || HasExt(p,"blend");
}

static bool IsImagePath(const std::string& p) {
    return HasExt(p,"png") || HasExt(p,"jpg") || HasExt(p,"jpeg") ||
           HasExt(p,"tga") || HasExt(p,"bmp") || HasExt(p,"psd")  ||
           HasExt(p,"gif") || HasExt(p,"hdr") || HasExt(p,"pic");
}

// Carga un modelo y lo deja delante de la cámara
static int AddModelFromFile(const char* path)
{
    SceneItem it;
    it.model = std::make_unique<Model>(path);
    // Colócalo ~2.5m frente a la cámara actual
    glm::vec3 front = camera.Front; // viene de tu Camera :contentReference[oaicite:2]{index=2}
    glm::vec3 pos   = camera.Position + front * 2.5f;
    it.M = glm::translate(glm::mat4(1.0f), pos);
    gScene.push_back(std::move(it));
    return (int)gScene.size()-1;
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

    SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_BEGIN, true);
    SDL_SetEventEnabled(SDL_EVENT_DROP_COMPLETE, true);

    SDL_SetWindowRelativeMouseMode(window, false);

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

    Shader shader("7.4.camera.vs", "7.4.camera.fs");

    Shader modelShader("1.model_loading.vs", "1.model_loading.fs");

    GLuint texHouse = LoadTextureDevIL("resources/objects/house/Baker_house.png");

    Model house("resources/objects/house/BakerHouse.fbx");

    house.SetOverrideTexture(texHouse);

    SceneItem first;
    first.model = std::make_unique<Model>("resources/objects/house/BakerHouse.fbx");
    first.overrideTex = texHouse;
    first.model->SetOverrideTexture(texHouse);
    first.M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    gScene.push_back(std::move(first));
    gSelectedIndex = 0;

    GLuint texture1 = LoadTextureDevIL("resources/textures/container.jpg");
    if (!texture1) SDL_Log("No se pudo cargar container.jpg");

    GLuint texture2 = LoadTextureDevIL("resources/textures/awesomeface.png");
    if (!texture2) SDL_Log("No se pudo cargar awesomeface.png");

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    auto cube = CreateCubeModel(1.0f, texture1);
    SceneItem cubeItem;
    cubeItem.overrideTex = texture1;
    cubeItem.model = std::move(cube);
    cubeItem.M = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    gScene.push_back(std::move(cubeItem));

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
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                    // por si sales mientras está capturado:
                    rightMouseHeld= false;
                    SDL_SetWindowRelativeMouseMode(window, false);
                } else if (event.key.key == SDLK_1) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else if (event.key.key == SDLK_2) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                    rightMouseHeld = false;
                    gOrbiting = false;
                    SDL_SetWindowRelativeMouseMode(window, false);
                } else if (event.key.key == SDLK_1) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else if (event.key.key == SDLK_2) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                } else if (event.key.key == SDLK_LALT || event.key.key == SDLK_RALT) {
                    altHeld = true;
                }
                else if (event.key.scancode == SDL_SCANCODE_F && gHasSelection && gSelectedIndex >= 0)
                {
                    AABB box = ComputeModelAABB(*gScene[gSelectedIndex].model, gScene[gSelectedIndex].M);
                    gOrbitTarget = 0.5f * (box.min + box.max);

                    float radius = glm::length(box.max - box.min) * 0.5f;
                    radius = glm::max(radius, 0.1f);

                    float distance = radius / tan(glm::radians(camera.Zoom * 0.5f));
                    gOrbitDistance = distance;

                    camera.Position = gOrbitTarget - camera.Front * distance;
                    SDL_Log("Zoom to selected (F): distance %.2f", distance);
                }
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            {
                int w, h;
                SDL_GetWindowSizeInPixels(window, &w, &h);
                glViewport(0, 0, w, h);
            } break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int w, h; SDL_GetWindowSizeInPixels(window, &w, &h);
                    glm::mat4 view = camera.GetViewMatrix();
                    float aspect = (h > 0) ? (float)w/(float)h : 4.0f/3.0f;
                    glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

                    glm::vec3 ro, rd;
                    BuildRayFromScreen(event.button.x, event.button.y, w, h, view, proj, ro, rd); // ya la tienes :contentReference[oaicite:6]{index=6}

                    float bestT = FLT_MAX;
                    int bestIdx = -1;
                    for (int i = 0; i < (int)gScene.size(); ++i) {
                        AABB box = ComputeModelAABB(*gScene[i].model, gScene[i].M);
                        float tHit;
                        if (RayIntersectsAABB(ro, rd, box, &tHit) && tHit < bestT) {
                            bestT = tHit; bestIdx = i;
                        }
                    }
                    gHasSelection = (bestIdx >= 0);
                    gSelectedIndex = bestIdx;
                    if (gHasSelection) {
                        AABB box = ComputeModelAABB(*gScene[bestIdx].model, gScene[bestIdx].M);
                        gOrbitTarget   = 0.5f * (box.min + box.max);
                        gOrbitDistance = glm::length(camera.Position - gOrbitTarget);
                        SDL_Log("Seleccionado item %d", bestIdx);
                    }
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    if (altHeld && gHasSelection) {
                        gOrbiting = true;
                        SDL_SetWindowRelativeMouseMode(window, true);   // captura para arrastrar sin límites
                    } else {
                        rightMouseHeld = true;                          // tu modo look FPS existente
                        SDL_SetWindowRelativeMouseMode(window, true);
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    gOrbiting = false;
                    rightMouseHeld = false;
                    SDL_SetWindowRelativeMouseMode(window, false);
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (gOrbiting && gHasSelection) {
                    float xoffset = (float)event.motion.xrel;
                    float yoffset = (float)event.motion.yrel;

                    // Actualiza yaw/pitch
                    camera.ProcessMouseMovement(xoffset, -yoffset);

                    // Recoloca la cámara para orbitar alrededor del target
                    // (Front ya viene de yaw/pitch en Camera)
                    camera.Position = gOrbitTarget - camera.Front * gOrbitDistance;
                }
                else if (rightMouseHeld) {
                    float xoffset = (float)event.motion.xrel;
                    float yoffset = (float)event.motion.yrel;
                    camera.ProcessMouseMovement(xoffset, -yoffset);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                if (gOrbiting && gHasSelection) {
                    // Dolly in/out cambiando la distancia orbital
                    float factor = 1.0f - 0.1f * (float)event.wheel.y; // 10% por notch
                    gOrbitDistance = glm::clamp(gOrbitDistance * factor, 0.5f, 200.0f);
                    camera.Position = gOrbitTarget - camera.Front * gOrbitDistance;
                } else if (rightMouseHeld) {
                    camera.ProcessMouseScroll((float)event.wheel.y);
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                rightMouseHeld = false;
                gOrbiting = false;
                SDL_SetWindowRelativeMouseMode(window, false);
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_LALT || event.key.key == SDLK_RALT) {
                    altHeld = false;
                }
                break;
            case SDL_EVENT_DROP_FILE:
                {
                    const char* dropped = event.drop.data;
                    if (!dropped) break;
                    std::string path = dropped;
                    SDL_Log("Dropped: %s", path.c_str());

                    if (IsModelPath(path)) {
                        int idx = AddModelFromFile(path.c_str());
                        if (idx >= 0) {
                            gSelectedIndex = idx;
                            // centra orbita en la AABB del nuevo modelo
                            AABB box = ComputeModelAABB(*gScene[idx].model, gScene[idx].M); // tu función ya existe :contentReference[oaicite:4]{index=4}
                            gHasSelection = true;
                            gOrbitTarget  = 0.5f * (box.min + box.max);
                            gOrbitDistance = glm::length(camera.Position - gOrbitTarget);
                        }
                    } else if (IsImagePath(path)) {
                        GLuint tex = LoadTextureDevIL(path.c_str(), /*mips=*/true);
                        if (tex) {
                            if (gSelectedIndex >= 0) {
                                gScene[gSelectedIndex].overrideTex = tex;
                                gScene[gSelectedIndex].model->SetOverrideTexture(tex); // soportado por tu Model :contentReference[oaicite:5]{index=5}
                            } else {
                                SDL_Log("Textura soltada sin selección; crea/selecciona un modelo para aplicarla.");
                            }
                        }
                    } else {
                        SDL_Log("Tipo de archivo no soportado para drop.");
                    }
                } break;

            }
        }

        const bool* ks = SDL_GetKeyboardState(NULL);
        if (rightMouseHeld) { // << opcional
            if (ks[SDL_SCANCODE_W]) camera.ProcessKeyboard(FORWARD,  deltaTime);
            if (ks[SDL_SCANCODE_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (ks[SDL_SCANCODE_A]) camera.ProcessKeyboard(LEFT,     deltaTime);
            if (ks[SDL_SCANCODE_D]) camera.ProcessKeyboard(RIGHT,    deltaTime);
        }


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

        glm::mat4 view = camera.GetViewMatrix();

        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        float aspect = (height > 0) ? (float)width / (float)height : 4.0f/3.0f;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

        // Sube a los uniforms como ya haces:
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

        modelShader.use();
        GLint mLoc = glGetUniformLocation(modelShader.ID, "model");
        GLint vLoc = glGetUniformLocation(modelShader.ID, "view");
        GLint pLoc = glGetUniformLocation(modelShader.ID, "projection");

        // mismas view/projection que ya calculas
        glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (size_t i = 0; i < gScene.size(); ++i) {
            glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(gScene[i].M));
            gScene[i].model->Draw(modelShader);
        }

        // INTERCAMBIAR buffers (mostrar en pantalla)
        SDL_GL_SwapWindow(window);

    }

    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);

    // Cleanup
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}