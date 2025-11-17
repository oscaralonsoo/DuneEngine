#pragma once

#include "Module.h"
#include "Mesh.h"
#include <vector>
#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>

struct CameraData
{
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::vec3 position = {0.0, 0.0, 0.0};
};

class ModuleRenderer : public Module
{
public:
	ModuleRenderer();
	~ModuleRenderer() = default;

	bool Start();

	bool PreUpdate();
	bool Update();
	bool PostUpdate();

	bool CleanUp();

private:
	CameraData cameraData;
};