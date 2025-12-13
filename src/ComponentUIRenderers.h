#pragma once

#include <memory>

class TransformComponent;
class MeshComponent;
class MaterialComponent;
class CameraComponent;
class ModuleInput;
class Texture;

namespace ComponentUI
{
    class TransformComponentUI
    {
    public:
        static void Render(TransformComponent* component, float panelWidth);
    };

    class MeshComponentUI
    {
    public:
        static void Render(MeshComponent* component, float panelWidth);
    };

    class MaterialComponentUI
    {
    public:
        static void Render(MaterialComponent* component, float panelWidth);

    private:
        static void DrawTextureSlot(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input);
        static void RenderSlotBorder(const char* name, std::shared_ptr<Texture>& texture);
        static void HandleSlotClick(const char* name, std::shared_ptr<Texture>& texture);
        static void RenderTexturePreview(const char* name, std::shared_ptr<Texture>& texture);
        static void HandleDragDrop(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input);
        static void HandleFileDrop(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input);
    };

    class CameraComponentUI
    {
    public:
        static void Render(CameraComponent* component, float panelWidth);
    };
}
