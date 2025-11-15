#pragma once

#include "Module.h"
#include "camera.h"

class ModuleCamera : public Module
{
public:
    ModuleCamera();
    ~ModuleCamera();

    bool Awake();
    bool Start();
    bool PreUpdate();
    bool Update();
    bool PostUpdate();
    bool CleanUp();
    
    Camera camera;
private:
};
