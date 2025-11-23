#pragma once

#include "Module.h"
#include "Mesh.h"
#include "EditorCamera.h"
#include "Texture.h"

#include <vector>
#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include "Shader.h"
#include <glad/glad.h>

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

	EditorCamera *renderCamera = nullptr; //FIXME -> Move to other class

private:
	Shader *shader = nullptr;
	std::shared_ptr<Texture> texture;
	glm::mat4 model, view, projection;
};