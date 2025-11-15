#pragma once
#include <list>
#include <memory>
#include "Module.h"

class ModuleWindow;
class ModuleInput;
class ModuleRender;
class ModuleCamera;
class ModuleTextures;
class ModuleImGui;
class ModuleEditor;

class Engine
{
public:
    static Engine &GetInstance();

    void AddModule(std::shared_ptr<Module> module);

    bool Awake();

    bool Start();

    bool Update();

    bool CleanUp();

	int Run();

private:
    Engine();

    Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	bool PreUpdate();
	bool DoUpdate();
	bool PostUpdate();

    std::list<std::shared_ptr<Module>> modules;

public:
	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

    std::shared_ptr<ModuleWindow> window;
    std::shared_ptr<ModuleInput> input;
    std::shared_ptr<ModuleRender> render;
    std::shared_ptr<ModuleCamera> camera;
    std::shared_ptr<ModuleTextures> textures;
	std::shared_ptr<ModuleImGui> imgui;
	std::shared_ptr<ModuleEditor> editor;
};
