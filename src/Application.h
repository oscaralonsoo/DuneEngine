#pragma once
#include <vector>
#include <memory>
#include "Module.h"

class Application
{
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void CleanUp();

    template<typename T>
    T* GetModule();

    bool running = true;
private:
    std::vector<std::unique_ptr<Module>> modules;
};
