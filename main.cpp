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
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <deque>
#include <chrono>
#include <SDL3/SDL_version.h>

static bool gShowConsole     = true;
static bool gShowConfig      = true;
static bool gShowHierarchy   = true;
static bool gShowInspector   = true;
static bool gShowAbout       = false;

static std::deque<float> gFpsHistory; // últimos N fps
static const int kFpsHistoryMax = 200;

static std::vector<std::string> gConsole; // consola
static ImGuiTextFilter gConsoleFilter;

static void DecomposeTRS(const glm::mat4& M, glm::vec3& outT, glm::vec3& outRdeg, glm::vec3& outS)
{
    // traslación
    outT = glm::vec3(M[3]);
    // escala
    glm::vec3 X = glm::vec3(M[0]);
    glm::vec3 Y = glm::vec3(M[1]);
    glm::vec3 Z = glm::vec3(M[2]);
    outS = glm::vec3(glm::length(X), glm::length(Y), glm::length(Z));
    if (outS.x != 0) X /= outS.x;
    if (outS.y != 0) Y /= outS.y;
    if (outS.z != 0) Z /= outS.z;
    // rotación (aprox, asumiendo sin shear; Euler YXZ)
    float yaw   = atan2f(X.z, X.x);
    float pitch = asinf(-X.y);
    float roll  = atan2f(Z.y, Y.y);
    outRdeg = glm::degrees(glm::vec3(pitch, yaw, roll));
}

// captura logs de SDL en la consola ImGui
static void SDLLogToConsole(void* /*userdata*/, int category, SDL_LogPriority priority, const char* message)
{
    char buf[2048];
    const char* pr =
        priority == SDL_LOG_PRIORITY_ERROR ? "[ERROR] " :
        priority == SDL_LOG_PRIORITY_WARN  ? "[WARN ] " :
        priority == SDL_LOG_PRIORITY_INFO  ? "[INFO ] " :
        priority == SDL_LOG_PRIORITY_DEBUG ? "[DEBUG] " : "[LOG  ] ";
    SDL_snprintf(buf, sizeof(buf), "%s%s", pr, message);
    gConsole.emplace_back(buf);
    if (gConsole.size() > 2000) gConsole.erase(gConsole.begin(), gConsole.begin() + 500);
}

static void PushFpsSample(float dt)
{
    float fps = (dt > 0.f) ? (1.f / dt) : 0.f;
    gFpsHistory.push_back(fps);
    if ((int)gFpsHistory.size() > kFpsHistoryMax) gFpsHistory.pop_back();
}

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
    std::string            name = "Item";
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
    glm::vec3 front = camera.Front;
    glm::vec3 pos   = camera.Position + front * 2.5f;
    it.M = glm::translate(glm::mat4(1.0f), pos);
    // nombre desde path
    std::filesystem::path p = std::filesystem::u8path(path);
    it.name = p.filename().string();
    gScene.push_back(std::move(it));
    return (int)gScene.size()-1;
}

static int PickItemAt(SDL_Window* window, int mx, int my)
{
    int w, h; SDL_GetWindowSizeInPixels(window, &w, &h);

    glm::mat4 view = camera.GetViewMatrix();
    float aspect   = (h > 0) ? (float)w/(float)h : 4.0f/3.0f;
    glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);

    glm::vec3 ro, rd;
    BuildRayFromScreen(mx, my, w, h, view, proj, ro, rd);

    float bestT = FLT_MAX;
    int bestIdx = -1;
    for (int i = 0; i < (int)gScene.size(); ++i) {
        AABB box = ComputeModelAABB(*gScene[i].model, gScene[i].M);
        float tHit;
        if (RayIntersectsAABB(ro, rd, box, &tHit) && tHit < bestT) {
            bestT = tHit; bestIdx = i;
        }
    }
    return bestIdx;
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
    SDL_Window *window = SDL_CreateWindow("DuneEngine", 1920, 1080, SDL_WINDOW_OPENGL);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

    ImGui::StyleColorsDark();

    // Inicializa backends (SDL3 + OpenGL3)
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Redirige logs de SDL a la consola UI
    SDL_SetLogOutputFunction(SDLLogToConsole, nullptr);

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

    Shader shader("Assets/shaders/Shader.vs", "Assets/shaders/Shader.fs");

    Shader modelShader("Assets/shaders/ModelShader.vs", "Assets/shaders/ModelShader.fs");

    GLuint texHouse = LoadTextureDevIL("Assets/objects/house/Baker_house.png");

    Model house("Assets/objects/house/BakerHouse.fbx");

    house.SetOverrideTexture(texHouse);

    SceneItem houseItem;
    houseItem.model = std::make_unique<Model>("Assets/objects/house/BakerHouse.fbx");
    houseItem.overrideTex = texHouse;
    houseItem.model->SetOverrideTexture(texHouse);
    houseItem.M = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -1.0f, 0.0f)); // más a la izquierda
    gScene.push_back(std::move(houseItem));
    houseItem.name = "House";

    SceneItem backpackItem;
    backpackItem.model = std::make_unique<Model>("Assets/objects/backpack/backpack.obj");
    backpackItem.M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)); // detrás, un poco separado
    gScene.push_back(std::move(backpackItem));

    GLuint texture1 = LoadTextureDevIL("Assets/textures/basic.jpg");
    if (!texture1) SDL_Log("No se pudo cargar container.jpg");
    backpackItem.name = "Backpack";

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    auto cube = CreateCubeModel(1.0f, texture1);
    SceneItem cubeItem;
    cubeItem.overrideTex = texture1;
    cubeItem.model = std::move(cube);
    cubeItem.M = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)); // más a la derecha
    gScene.push_back(std::move(cubeItem));

    cubeItem.name = "Cube";

    bool running = true;

    lastFrame = SDL_GetTicks() / 1000.0f;

    while (running)
    {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* vp = ImGui::GetMainViewport();

        // Ventana invisible a pantalla completa que aloja el DockSpace
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground; // deja ver tu escena
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("##DockSpaceHost", nullptr, host_flags);
        ImGui::PopStyleVar(2);

        // Crea el DockSpace
        ImGuiID dockspace_id = ImGui::GetID("DuneEngineDockSpace");
        ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode; 
        ImGui::DockSpace(dockspace_id, ImVec2(0,0), dock_flags);
        ImGui::End();

        // Construye el layout SOLO la primera vez (si no hay .ini previo)
        static bool s_built = false;
        if (!s_built && ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            s_built = true;

            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | dock_flags);
            ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);

            // Particiones: Left (jerarquía), Right (inspector+config), Down (console), Center (viewport)
            ImGuiID dock_main   = dockspace_id;
            ImGuiID dock_left   = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left,  0.20f, nullptr, &dock_main);
            ImGuiID dock_right  = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.28f, nullptr, &dock_main);
            ImGuiID dock_down   = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down,  0.28f, nullptr, &dock_main);
            ImGuiID dock_top    = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up,    0.08f, nullptr, &dock_main);

            // Ubica ventanas por nombre (deben existir con esos títulos)
            ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_right);
            ImGui::DockBuilderDockWindow("Config",    dock_right);
            ImGui::DockBuilderDockWindow("Console",   dock_down);
            ImGui::DockBuilderDockWindow("Toolbar",   dock_top);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        if (gShowConsole)
        {
            ImGui::Begin("Console", &gShowConsole);
            gConsoleFilter.Draw("Filtro");
            ImGui::Separator();
            ImGui::BeginChild("ConsoleScroll", ImVec2(0,0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            for (auto& line : gConsole)
                if (gConsoleFilter.PassFilter(line.c_str()))
                    ImGui::TextUnformatted(line.c_str());
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
            ImGui::End();
        }
        if (gShowInspector)
        {
            ImGui::Begin("Inspector", &gShowInspector);
            if (gHasSelection && gSelectedIndex >= 0)
            {
                SceneItem& it = gScene[gSelectedIndex];

                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    glm::vec3 T, Rdeg, S;
                    DecomposeTRS(it.M, T, Rdeg, S);
                    ImGui::Text("Position:  (%.3f, %.3f, %.3f)", T.x, T.y, T.z);
                    ImGui::Text("Rotation:  (%.1f, %.1f, %.1f) deg", Rdeg.x, Rdeg.y, Rdeg.z);
                    ImGui::Text("Scale:     (%.3f, %.3f, %.3f)", S.x, S.y, S.z);
                }

                if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    size_t totalVerts = 0, totalIdx = 0;
                    for (auto& m : it.model->meshes) { totalVerts += m.vertices.size(); totalIdx += m.indices.size(); }
                    ImGui::Text("Meshes: %d", (int)it.model->meshes.size());
                    ImGui::Text("Vertices: %d", (int)totalVerts);
                    ImGui::Text("Indices: %d", (int)totalIdx);

                    static bool showNormalsFace   = false; // TODO: implementar visualización
                    static bool showNormalsVertex = false; // TODO: implementar visualización
                    ImGui::Checkbox("Mostrar normales por cara (TODO)", &showNormalsFace);
                    ImGui::Checkbox("Mostrar normales por vértice (TODO)", &showNormalsVertex);
                }

                if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    // recoge primera textura si existe
                    GLuint texId = 0;
                    std::string path = "(none)";
                    if (!it.model->meshes.empty() && !it.model->meshes[0].textures.empty())
                    {
                        texId = it.model->meshes[0].textures[0].id;
                        path  = it.model->meshes[0].textures[0].path;
                    }
                    ImGui::Text("Path: %s", path.c_str());

                    if (texId != 0)
                    {
                        GLint w=0, h=0;
                        glBindTexture(GL_TEXTURE_2D, texId);
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &w);
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
                        ImGui::Text("Size: %dx%d", w, h);

                        // checker preview toggle
                        static bool showChecker = false;
                        ImGui::Checkbox("Visualizar tablero blanco/negro (preview UI)", &showChecker);
                        if (showChecker)
                        {
                            // Dibujamos un checker en la UI (no en el objeto) como indicador
                            ImVec2 size(128, 128);
                            ImDrawList* dl = ImGui::GetWindowDrawList();
                            ImVec2 p = ImGui::GetCursorScreenPos();
                            const int cells = 8;
                            for (int y = 0; y < cells; ++y)
                            for (int x = 0; x < cells; ++x)
                            {
                                ImU32 col = ((x+y)&1) ? IM_COL32(220,220,220,255) : IM_COL32(40,40,40,255);
                                ImVec2 a = ImVec2(p.x + x*size.x/cells, p.y + y*size.y/cells);
                                ImVec2 b = ImVec2(p.x + (x+1)*size.x/cells, p.y + (y+1)*size.y/cells);
                                dl->AddRectFilled(a, b, col);
                            }
                            ImGui::Dummy(size);
                        }

                        // mini preview de la textura real
                        ImGui::Text("Preview:");
                        ImGui::Image((ImTextureID)(intptr_t)texId, ImVec2(128,128));
                    }
                }
            }
            else {
                ImGui::Text("No hay selección.");
            }
            ImGui::End();
        }

        if (gShowHierarchy)
        {
            ImGui::Begin("Hierarchy", &gShowHierarchy);
            for (int i = 0; i < (int)gScene.size(); ++i)
            {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (i == gSelectedIndex) flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s##%d", gScene[i].name.c_str(), i);
                if (ImGui::IsItemClicked())
                {
                    gSelectedIndex = i;
                    gHasSelection = true;
                    AABB box = ComputeModelAABB(*gScene[i].model, gScene[i].M);
                    gOrbitTarget   = 0.5f * (box.min + box.max);
                    gOrbitDistance = glm::length(camera.Position - gOrbitTarget);
                }
            }
            ImGui::End();
        }


        if (gShowConfig)
        {
            ImGui::Begin("Config", &gShowConfig);
            // FPS graph
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            if (!gFpsHistory.empty()) {
                static std::vector<float> fpsTmp;
                fpsTmp.assign(gFpsHistory.begin(), gFpsHistory.end());
                ImGui::PlotLines("##fps", fpsTmp.data(), (int)fpsTmp.size(), 0, nullptr, 0.0f, 200.0f, ImVec2(-1, 80));
            }

            ImGui::Separator();
            // Info HW/SW
            ImGui::Text("GPU Vendor: %s", (const char*)glGetString(GL_VENDOR));
            ImGui::Text("GPU Renderer: %s", (const char*)glGetString(GL_RENDERER));
            ImGui::Text("OpenGL: %s", (const char*)glGetString(GL_VERSION));
            ImGui::Text("GLSL: %s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

            // (Simple) Memoria aprox: solo mostramos tamaño de escena como ejemplo
            ImGui::Separator();
            ImGui::Text("Scene items: %d", (int)gScene.size());
            ImGui::End();
        }



        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit", "Esc")) {
                    // emula tu salida ordenada
                    SDL_Event quit{}; quit.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&quit);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Console",     nullptr, &gShowConsole);
                ImGui::MenuItem("Config",      nullptr, &gShowConfig);
                ImGui::MenuItem("Hierarchy",   nullptr, &gShowHierarchy);
                ImGui::MenuItem("Inspector",   nullptr, &gShowInspector);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Documentación del GitHub")) {
                    SDL_OpenURL("https://github.com/oscaralonsoo/docs");
                }
                if (ImGui::MenuItem("Reportar un bug")) {
                    SDL_OpenURL("https://github.com/oscaralonsoo/issues");
                }
                if (ImGui::MenuItem("Descargar último")) {
                    SDL_OpenURL("https://github.com/oscaralonsoo/releases");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("About")) gShowAbout = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Toolbar simple
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Toolbar", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextUnformatted("Primitives:");
        ImGui::SameLine();
        if (ImGui::Button("Add Cube"))
        {
            auto cube = CreateCubeModel(1.0f, 0);
            SceneItem item;
            item.model = std::move(cube);
            item.name = "Cube";
            // colócalo frente a la cámara
            glm::vec3 pos = camera.Position + camera.Front * 2.5f;
            item.M = glm::translate(glm::mat4(1.0f), pos);
            gScene.push_back(std::move(item));
        }
        ImGui::End();

        // --- About modal ---
        if (gShowAbout) {
            ImGui::OpenPopup("About DuneEngine");
        }
        if (ImGui::BeginPopupModal("About DuneEngine", &gShowAbout, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("DuneEngine - v0.1");
            ImGui::Separator();
            ImGui::Text("Equipo: ... (añade nombres + GitHubs)");
            ImGui::Text("Librerías: SDL3, GLAD, GLM, DevIL, Assimp, ImGui");
            ImGui::Text("Licencia: MIT");
            if (ImGui::Button("Cerrar")) { gShowAbout = false; ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }


        if (deltaTime > 0.05f) deltaTime = 0.05f;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
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
                else if (gHasSelection && gSelectedIndex >= 0) {
                    // Escalar
                    if (event.key.key == SDLK_UP) {
                        gScene[gSelectedIndex].M =
                            glm::scale(gScene[gSelectedIndex].M, glm::vec3(1.05f));
                    }
                    else if (event.key.key == SDLK_DOWN) {
                        gScene[gSelectedIndex].M =
                            glm::scale(gScene[gSelectedIndex].M, glm::vec3(0.95f));
                    }
                    // Rotar
                    else if (event.key.key == SDLK_LEFT) {
                        gScene[gSelectedIndex].M =
                            glm::rotate(gScene[gSelectedIndex].M, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 5° izquierda
                    }
                    else if (event.key.key == SDLK_RIGHT) {
                        gScene[gSelectedIndex].M =
                            glm::rotate(gScene[gSelectedIndex].M, glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 5° derecha
                    }
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
                const char* dropped = event.drop.data;
                if (!dropped) break;
                std::string path = dropped;
                SDL_Log("Dropped: %s", path.c_str());

                if (IsModelPath(path)) {
                    int idx = AddModelFromFile(path.c_str());
                    if (idx >= 0) {
                        gSelectedIndex = idx;
                        AABB box = ComputeModelAABB(*gScene[idx].model, gScene[idx].M);
                        gHasSelection = true;
                        gOrbitTarget  = 0.5f * (box.min + box.max);
                        gOrbitDistance = glm::length(camera.Position - gOrbitTarget);
                    }
                } else if (IsImagePath(path)) {
                    // 1) Cargamos textura
                    GLuint tex = LoadTextureDevIL(path.c_str(), /*mips=*/true);
                    if (tex) {
                        // 2) Averiguamos el objeto bajo el cursor de drop
                        //    SDL3 expone event.drop.x / event.drop.y en coords de ventana.
                        int dropX = event.drop.x;
                        int dropY = event.drop.y;

                        int idx = PickItemAt(window, dropX, dropY);

                        // 3) Fallback al seleccionado si no había ninguno bajo el cursor
                        if (idx < 0 && gSelectedIndex >= 0) idx = gSelectedIndex;

                        if (idx >= 0) {
                            gScene[idx].model->SetOverrideTexture(tex); // ahora sí fuerza el uso
                            gSelectedIndex = idx;
                            gHasSelection  = true;
                            SDL_Log("Textura aplicada al item %d", idx);
                        } else {
                            SDL_Log("Textura soltada sin ningún objeto debajo.");
                        }
                    }
                } else {
                    SDL_Log("Tipo de archivo no soportado para drop.");
                }
                break;

            }
        }

        const bool* ks = SDL_GetKeyboardState(NULL);
        if (rightMouseHeld) {
            float speedMultiplier = ks[SDL_SCANCODE_LSHIFT] || ks[SDL_SCANCODE_RSHIFT] ? 2.0f : 1.0f;

            if (ks[SDL_SCANCODE_W]) camera.ProcessKeyboard(FORWARD,  deltaTime * speedMultiplier);
            if (ks[SDL_SCANCODE_S]) camera.ProcessKeyboard(BACKWARD, deltaTime * speedMultiplier);
            if (ks[SDL_SCANCODE_A]) camera.ProcessKeyboard(LEFT,     deltaTime * speedMultiplier);
            if (ks[SDL_SCANCODE_D]) camera.ProcessKeyboard(RIGHT,    deltaTime * speedMultiplier);
        }


        if (gHasSelection && gSelectedIndex >= 0 && !rightMouseHeld) {
            float moveSpeed = 2.0f * deltaTime; // velocidad de movimiento
            glm::vec3 translation(0.0f);

            // Movimiento en el plano XY: W↑, S↓, A←, D→
            if (ks[SDL_SCANCODE_W]) translation.y += moveSpeed;
            if (ks[SDL_SCANCODE_S]) translation.y -= moveSpeed;
            if (ks[SDL_SCANCODE_A]) translation.x -= moveSpeed;
            if (ks[SDL_SCANCODE_D]) translation.x += moveSpeed;
            if (ks[SDL_SCANCODE_Q]) translation.z -= moveSpeed; 
            if (ks[SDL_SCANCODE_E]) translation.z += moveSpeed; 

            // Aplica la traslación al objeto seleccionado
            gScene[gSelectedIndex].M = glm::translate(gScene[gSelectedIndex].M, translation);
        }

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

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

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

    }

    glDeleteTextures(1, &texture1);

    // Cleanup
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}