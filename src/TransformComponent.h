#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include "GameObject.h"

class TransformComponent : public Component
{
public:
    TransformComponent(GameObject *owner);
    ~TransformComponent() = default;

    const glm::vec3 &GetPosition() const { return position; }
    const glm::vec3 &GetRotation() const { return rotation; }
    const glm::vec3 &GetScale() const { return scale; }

    void SetPosition(const glm::vec3 &p) { position = p; UpdateWorldMatrix(); }
    void SetRotation(const glm::vec3 &r) { rotation = r; UpdateWorldMatrix(); }
    void SetScale(const glm::vec3 &s)   { scale = s; UpdateWorldMatrix(); }

    const glm::mat4 &GetWorldTransform() const { return mWorldMatrix; }
    void SetWorldTransform(const glm::mat4 &t) { mWorldMatrix = t; }

    void OnInspectorRender(float panelWidth) override;

private:
    void UpdateWorldMatrix();
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 mWorldMatrix = glm::mat4(1.0f);
};
