#pragma once
#include <string>

class Application;

class Module
{
public:
    Module(Application* app, const std::string& name) : app(app), name(name) {}
    virtual ~Module() = default;

    virtual bool Awake() { return true; }
    virtual bool Start() { return true; }
    virtual bool PreUpdate() { return true; }
    virtual bool Update() { return true; }
    virtual bool PostUpdate() { return true; }
    virtual bool CleanUp() { return true; }

    std::string GetName() const { return name; }

protected:
    Application* app = nullptr;
    std::string name;
};
