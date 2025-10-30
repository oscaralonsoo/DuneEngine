#pragma once
#include <memory>

class Window;
class Shader;
class Mesh;

class App {
public:
    bool init();
    void run();
    void shutdown();

private:
    bool m_running = false;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Mesh>   m_mesh;
};
