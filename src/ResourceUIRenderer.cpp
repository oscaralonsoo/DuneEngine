#include "ResourceUIRenderer.h"
#include "Engine.h"
#include "ModuleResource.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace ResourceUI
{
    std::string FormatMemorySize(size_t bytes)
    {
        const char* units[] = { "B", "KB", "MB", "GB" };
        int unitIndex = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unitIndex < 3)
        {
            size /= 1024.0;
            unitIndex++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return oss.str();
    }

    std::string GetResourceTypeName(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Texture: return "Texture";
        case ResourceType::Model: return "Model";
        case ResourceType::Mesh: return "Mesh";
        case ResourceType::Material: return "Material";
        case ResourceType::Shader: return "Shader";
        case ResourceType::Cubemap: return "Cubemap";
        case ResourceType::Prefab: return "Prefab";
        default: return "Unknown";
        }
    }

    void RenderResourceDetails(
        std::shared_ptr<Resource> resource,
        const std::filesystem::path& assetPath)
    {
        if (!resource)
            return;
        
        ResourceType type = resource->GetType();
        std::string filename = assetPath.filename().string();
        
        // Render header
        RenderResourceHeader(filename);
        
        // Render resource info section
        RenderResourceInfo(resource, assetPath, type);
        ImGui::Spacing();
        
        // Render details section
        if (ImGui::CollapsingHeader("Details", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent(10.0f);
            
            // File size on disk
            if (std::filesystem::exists(assetPath))
            {
                size_t fileSize = std::filesystem::file_size(assetPath);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("File Size (Disk)");
                ImGui::SameLine(140.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
                ImGui::Text("%s", FormatMemorySize(fileSize).c_str());
                ImGui::PopStyleColor();
            }
            
            // Memory size (uncompressed)
            size_t memSize = resource->GetMemorySize();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Memory Size (RAM)");
            ImGui::SameLine(140.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
            ImGui::Text("%s", FormatMemorySize(memSize).c_str());
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Type-specific details
            switch (type)
            {
                case ResourceType::Texture:
                    RenderTextureDetails(std::dynamic_pointer_cast<Texture>(resource));
                    break;
                case ResourceType::Mesh:
                    RenderMeshDetails(std::dynamic_pointer_cast<Mesh>(resource));
                    break;
                case ResourceType::Material:
                    RenderMaterialDetails(std::dynamic_pointer_cast<Material>(resource));
                    break;
                case ResourceType::Model:
                    RenderModelDetails(std::dynamic_pointer_cast<Model>(resource));
                    break;
                default:
                    break;
            }
            
            ImGui::Unindent(10.0f);
        }
        
        ImGui::Spacing();
        
        // Render usage info
        RenderUsageInfo(resource);
    }

    void RenderResourceHeader(const std::string& filename)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("%s", filename.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    void RenderResourceInfo(
        std::shared_ptr<Resource> resource,
        const std::filesystem::path& assetPath,
        ResourceType type)
    {
        if (ImGui::CollapsingHeader("Resource Info", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent(10.0f);
            
            // UID
            ImGui::AlignTextToFramePadding();
            ImGui::Text("UID");
            ImGui::SameLine(120.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 1.0f, 1.0f));
            ImGui::Text("%s", resource->GetUID().ToString().c_str());
            ImGui::PopStyleColor();
            
            // Type
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Type");
            ImGui::SameLine(120.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
            ImGui::Text("%s", GetResourceTypeName(type).c_str());
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Paths section
            ImGui::Text("Paths");
            ImGui::Spacing();
            
            ImGui::Text("  Asset:");
            ImGui::Indent(20.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextWrapped("%s", assetPath.string().c_str());
            ImGui::PopStyleColor();
            ImGui::Unindent(20.0f);
            
            std::string libPath = resource->GetLibraryPath().empty() ? "N/A" : resource->GetLibraryPath().string();
            ImGui::Text("  Library:");
            ImGui::Indent(20.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextWrapped("%s", libPath.c_str());
            ImGui::PopStyleColor();
            ImGui::Unindent(20.0f);
            
            std::filesystem::path metaPath = assetPath;
            metaPath += ".meta";
            ImGui::Text("  Meta:");
            ImGui::Indent(20.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextWrapped("%s", metaPath.string().c_str());
            ImGui::PopStyleColor();
            ImGui::Unindent(20.0f);
            
            ImGui::Unindent(10.0f);
        }
    }

    void RenderTextureDetails(std::shared_ptr<Texture> texture)
    {
        if (!texture)
            return;
        
        ImGui::Text("Texture Properties");
        ImGui::Spacing();
        ImGui::Indent(10.0f);
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Dimensions");
        ImGui::SameLine(120.0f);
        ImGui::TextColored(ImVec4(0.6f, 0.9f, 1.0f, 1.0f), "%u x %u px", texture->GetWidth(), texture->GetHeight());
        
        ImGui::Unindent(10.0f);
    }

    void RenderMeshDetails(std::shared_ptr<Mesh> mesh)
    {
        if (!mesh)
            return;
        
        ImGui::Text("Mesh Properties");
        ImGui::Spacing();
        ImGui::Indent(10.0f);
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Vertices");
        ImGui::SameLine(120.0f);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "%zu", mesh->GetVertices().size());
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Triangles");
        ImGui::SameLine(120.0f);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "%zu", mesh->GetIndices().size() / 3);
        
        ImGui::Unindent(10.0f);
    }

    void RenderMaterialDetails(std::shared_ptr<Material> material)
    {
        if (!material)
            return;
        
        ImGui::Text("Material Properties");
        ImGui::Spacing();
        ImGui::Indent(10.0f);
        
        const auto& textures = material->GetTextures();
        ImGui::Text("Texture Maps:");
        ImGui::Spacing();
        
        if (textures.albedo) 
        {
            ImGui::BulletText("Albedo");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Base Color)");
        }
        if (textures.normal) 
        {
            ImGui::BulletText("Normal");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Surface Detail)");
        }
        if (textures.metallic) 
        {
            ImGui::BulletText("Metallic");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Metal/Non-metal)");
        }
        if (textures.roughness) 
        {
            ImGui::BulletText("Roughness");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Surface Smoothness)");
        }
        if (textures.ao) 
        {
            ImGui::BulletText("Ambient Occlusion");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Shadows)");
        }
        if (textures.emissive) 
        {
            ImGui::BulletText("Emissive");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Glow)");
        }
        
        if (!textures.albedo && !textures.normal && !textures.metallic && 
            !textures.roughness && !textures.ao && !textures.emissive)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "  No textures assigned");
        }
        
        ImGui::Unindent(10.0f);
    }

    void RenderModelDetails(std::shared_ptr<Model> model)
    {
        if (!model)
            return;
        
        ImGui::Text("Model Properties");
        ImGui::Spacing();
        ImGui::Indent(10.0f);
        
        const auto& meshes = model->GetMeshes();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Total Meshes");
        ImGui::SameLine(120.0f);
        ImGui::TextColored(ImVec4(0.6f, 0.9f, 1.0f, 1.0f), "%zu", meshes.size());
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("Mesh Breakdown:");
        ImGui::Spacing();
        
        for (size_t i = 0; i < meshes.size(); ++i)
        {
            ImGui::PushID((int)i);
            
            // Create a colored header for each mesh
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.3f, 0.4f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.4f, 0.5f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.35f, 0.45f, 0.8f));
            
            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, 
                ImGuiTreeNodeFlags_SpanAvailWidth, 
                "Mesh %zu", i);
            
            ImGui::PopStyleColor(3);
            
            if (nodeOpen)
            {
                const auto& mesh = meshes[i];
                ImGui::Spacing();
                ImGui::Indent(10.0f);
                
                // Vertices
                ImGui::AlignTextToFramePadding();
                ImGui::Text("▸ Vertices");
                ImGui::SameLine(110.0f);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "%zu", mesh->GetVertices().size());
                
                // Triangles
                ImGui::AlignTextToFramePadding();
                ImGui::Text("▸ Triangles");
                ImGui::SameLine(110.0f);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "%zu", mesh->GetIndices().size() / 3);
                
                // Indices
                ImGui::AlignTextToFramePadding();
                ImGui::Text("▸ Indices");
                ImGui::SameLine(110.0f);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "%zu", mesh->GetIndices().size());
                
                // Memory
                size_t meshMemory = mesh->GetMemorySize();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("▸ Memory");
                ImGui::SameLine(110.0f);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", FormatMemorySize(meshMemory).c_str());
                
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
                ImGui::TreePop();
            }
            
            ImGui::PopID();
        }
        
        ImGui::Unindent(10.0f);
    }

    void RenderUsageInfo(std::shared_ptr<Resource> resource)
    {
        if (ImGui::CollapsingHeader("Usage", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent(10.0f);
            
            int usageCount = Engine::GetInstance().resourceManager->GetResourceUsageCount(resource->GetUID());
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Used by GameObjects");
            ImGui::SameLine(180.0f);
            
            if (usageCount > 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
                ImGui::Text("%d", usageCount);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), usageCount == 1 ? "object" : "objects");
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                ImGui::Text("0");
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Not in use)");
            }
            
            ImGui::Unindent(10.0f);
        }
    }
}
