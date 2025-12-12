#include "ModuleImGui.h"
#include "Engine.h"
#include "ModuleWindow.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

ModuleImGui::ModuleImGui()
{
    name = "imgui";
}

ModuleImGui::~ModuleImGui()
{
}

bool ModuleImGui::Awake()
{
    return true;
}

bool ModuleImGui::Start()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::LoadIniSettingsFromDisk("imgui.ini");

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    // Apply desert theme
    style.Colors[ImGuiCol_Text] = ImVec4(0.40f, 0.32f, 0.23f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.82f, 0.71f, 0.55f, 1.00f); 
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.80f, 0.65f, 1.00f); 
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.75f, 0.60f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.72f, 0.67f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.77f, 0.72f, 0.63f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.55f, 0.50f, 0.42f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.82f, 0.71f, 0.55f, 0.30f); 
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.80f, 0.65f, 0.30f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.75f, 0.60f, 0.45f, 0.70f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.82f, 0.71f, 0.55f, 1.00f); 
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.75f, 0.60f, 0.45f, 1.00f); 
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.71f, 0.55f, 0.50f); 
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.24f, 0.17f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.38f, 0.30f, 1.00f); 
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.38f, 0.32f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.82f, 0.71f, 0.55f, 1.00f); 
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.90f, 0.80f, 0.65f, 1.00f); 
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.75f, 0.60f, 0.45f, 1.00f); 
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.82f, 0.71f, 0.55f, 0.50f); 
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.75f, 0.60f, 0.45f, 0.50f); 
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.90f, 0.80f, 0.65f, 0.70f);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Engine& engine = Engine::GetInstance();
    SDL_Window* window = engine.window->GetWindow();
    SDL_GLContext glContext = engine.window->GetGLContext();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

bool ModuleImGui::PreUpdate()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    return true;
}

bool ModuleImGui::Update()
{
    return true;
}

bool ModuleImGui::PostUpdate()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backupWindow = SDL_GL_GetCurrentWindow();
        SDL_GLContext backupContext = SDL_GL_GetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        SDL_GL_MakeCurrent(backupWindow, backupContext);
    }

    SDL_GL_SwapWindow(Engine::GetInstance().window->GetWindow());

    return true;
}

bool ModuleImGui::CleanUp()
{
    ImGui::SaveIniSettingsToDisk("imgui.ini");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    return true;
}
