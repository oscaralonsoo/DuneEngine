#include "SceneAssetHandler.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ResourceUtils.h"
#include "GameObject.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Material.h"
#include "Texture.h"
#include "Globals.h"
#include <memory>

void SceneAssetHandler::HandleDragDrop(ImVec2 viewSize, float mouseX, float mouseY, Framebuffer* framebuffer)
{
    uint32_t width = framebuffer ? framebuffer->GetWidth() : 0;
    uint32_t height = framebuffer ? framebuffer->GetHeight() : 0;
    
    const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload();
    bool isDraggingTexture = false;
    
    if (dragPayload && dragPayload->IsDataType("ASSET_PAYLOAD"))
    {
        const char* relativePath = (const char*)dragPayload->Data;
        if (relativePath && relativePath[0] != '\0')
        {
            std::filesystem::path fullPath = "Assets" / std::filesystem::path(relativePath);
            ResourceType type = ResourceUtils::GetTypeFromExtension(fullPath);
            isDraggingTexture = (type == ResourceType::Texture);
        }
    }

    if (isDraggingTexture && mouseX >= 0.0f && mouseY >= 0.0f)
    {
        UpdateHoverState(mouseX, mouseY, width, height, true);
    }
    else
    {
        UpdateHoverState(-1.0f, -1.0f, 0, 0, false);
    }

    // Handle drop
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
        {
            const char* relativePath = (const char*)payload->Data;
            if (relativePath && relativePath[0] != '\0')
            {
                std::filesystem::path fullPath = "Assets" / std::filesystem::path(relativePath);
                HandleAssetDrop(fullPath, mouseX, mouseY, width, height);
            }
            ClearHoverState();
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* path = (const char*)payload->Data;
            if (path && path[0] != '\0')
            {
                std::filesystem::path assetPath = path;
                HandleAssetDrop(assetPath, mouseX, mouseY, width, height);
            }
            ClearHoverState();
        }
        ImGui::EndDragDropTarget();
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
    
    if (m_HoveredObject)
    {
        hoveredObject = m_HoveredObject;
    }

    else if (mouseX >= 0.0f && mouseY >= 0.0f && width > 0 && height > 0)
    {
        auto picked = scene->GetRaycaster()->PickObject(mouseX, mouseY, width, height, scene->GetGameObjects());
        
        if (picked)
        {
            GameObject* pickedObj = picked.get();
            
            // Apply texture directly to the picked object if it has MaterialComponent
            auto* matComp = pickedObj->GetComponent<MaterialComponent>();
            if (matComp && matComp->GetMaterial())
            {
                hoveredObject = pickedObj;
            }
        }
    }

    switch (type)
    {
        case ResourceType::Texture:
            if (hoveredObject)
            {
                ApplyTextureToObject(hoveredObject, assetPath);
            }
            return;

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

    material->SetTexture(TextureType::Albedo, std::make_shared<Texture>(texturePath));
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

void SceneAssetHandler::UpdateHoverState(float mouseX, float mouseY, uint32_t width, uint32_t height, bool isDraggingTexture)
{
    m_IsTextureDragging = isDraggingTexture;

    if (!isDraggingTexture || mouseX < 0.0f || mouseY < 0.0f || width == 0 || height == 0)
    {
        m_HoveredObject = nullptr;
        return;
    }

    auto* scene = Engine::GetInstance().scene.get();
    if (!scene)
    {
        m_HoveredObject = nullptr;
        return;
    }

    auto picked = scene->GetRaycaster()->PickObject(mouseX, mouseY, width, height, scene->GetGameObjects());
    
    if (picked)
    {
        GameObject* pickedObj = picked.get();
        auto* matComp = pickedObj->GetComponent<MaterialComponent>();
        
        if (matComp && matComp->GetMaterial())
        {
            m_HoveredObject = pickedObj;
        }
        else
        {
            m_HoveredObject = nullptr;
        }
        return;
    }
    
    m_HoveredObject = nullptr;
}

void SceneAssetHandler::ClearHoverState()
{
    m_HoveredObject = nullptr;
    m_IsTextureDragging = false;
}
