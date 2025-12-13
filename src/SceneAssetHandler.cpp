#include "SceneAssetHandler.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ResourceUtils.h"
#include "GameObject.h"
#include "MaterialComponent.h"
#include "Material.h"
#include "Texture.h"
#include <memory>

void SceneAssetHandler::HandleDragDrop(ImVec2 viewSize, float mouseX, float mouseY, Framebuffer* framebuffer)
{
    if (ImGui::BeginDragDropTarget())
    {
        // Handle assets from ProjectPanel
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            const char* relativePath = (const char*)payload->Data;
            if (relativePath && relativePath[0] != '\0')
            {
                std::filesystem::path fullPath = "Assets" / std::filesystem::path(relativePath);
                uint32_t width = framebuffer ? framebuffer->GetWidth() : 0;
                uint32_t height = framebuffer ? framebuffer->GetHeight() : 0;
                HandleAssetDrop(fullPath, mouseX, mouseY, width, height);
            }
        }
        // Handle external files from File Explorer
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* path = (const char*)payload->Data;
            if (path && path[0] != '\0')
            {
                std::filesystem::path assetPath = path;
                uint32_t width = framebuffer ? framebuffer->GetWidth() : 0;
                uint32_t height = framebuffer ? framebuffer->GetHeight() : 0;
                HandleAssetDrop(assetPath, mouseX, mouseY, width, height);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Also handle file drops via ModuleInput
    auto* input = Engine::GetInstance().input.get();
    if (input && input->WasFileDropped())
    {
        std::string path = input->GetDraggedFile();
        if (!path.empty())
        {
            HandleAssetDrop(std::filesystem::path(path), -1.0f, -1.0f, 0, 0);
            input->ClearDropState();
            input->ClearDraggedFile();
        }
    }
}

void SceneAssetHandler::HandleAssetDrop(const std::filesystem::path& assetPath, float mouseX, float mouseY, 
                                       uint32_t width, uint32_t height)
{
    auto* scene = Engine::GetInstance().scene.get();
    if (!scene)
        return;

    ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);

    GameObject* hoveredObject = nullptr;
    if (mouseX >= 0.0f && mouseY >= 0.0f && width > 0 && height > 0)
    {
        auto picked = scene->GetRaycaster()->PickObject(mouseX, mouseY, width, height, scene->GetGameObjects());
        hoveredObject = picked.get();
    }

    switch (type)
    {
        case ResourceType::Unknown:
            return;

        case ResourceType::Texture:
        {
            if (hoveredObject)
            {
                ApplyTextureToObject(hoveredObject, assetPath);
            }
            return;
        }

        case ResourceType::Model:
            InstantiateModel(assetPath);
            return;

        case ResourceType::Prefab:
            InstantiatePrefab(assetPath);
            return;

        default:
            return;
    }
}

void SceneAssetHandler::ApplyTextureToObject(GameObject* object, const std::filesystem::path& texturePath)
{
    if (!object)
        return;

    auto* matComp = object->GetComponent<MaterialComponent>();
    if (!matComp)
        return;

    auto material = matComp->GetMaterial();
    if (!material)
        return;

    material->GetTextures().albedo = std::make_shared<Texture>(texturePath);
    matComp->SetMaterial(material);
}

void SceneAssetHandler::InstantiateModel(const std::filesystem::path& modelPath)
{
    auto* scene = Engine::GetInstance().scene.get();
    if (scene)
    {
        scene->CreateGameObjectFromModel(modelPath);
    }
}

void SceneAssetHandler::InstantiatePrefab(const std::filesystem::path& prefabPath)
{
    auto* scene = Engine::GetInstance().scene.get();
    if (scene)
    {
        scene->CreateGameObjectFromPrefab(prefabPath);
    }
}
