#pragma once

#include "Component.h"
#include <glm/glm.hpp>

class TransformComponent : public Component
{
public:
	TransformComponent(GameObject* owner);
	~TransformComponent() = default;

	const glm::vec3& GetPosition() const { return position; }
	const glm::vec3& GetRotation() const { return rotation; }
	const glm::vec3& GetScale() const { return scale; }

	void SetPosition(const glm::vec3& p) { position = p; }
	void SetRotation(const glm::vec3& r) { rotation = r; }
	void SetScale(const glm::vec3& s) { scale = s; }

private:
	glm::vec3 position{0.0f, 0.0f, 0.0f};
	glm::vec3 rotation{0.0f, 0.0f, 0.0f};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
};