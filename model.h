#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"        
#include "shader_s.h"    

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>
using namespace std;

#include <filesystem>

// Usa tu cargador de DevIL del main.cpp
extern unsigned int LoadTextureDevIL(const char* path, bool genMipmaps);

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

class Model 
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
    void SetOverrideTexture(unsigned int tex)
    {
        overrideTexture = tex;

        // Fuerza la textura en todas las meshes ya construidas
        for (auto& m : meshes) {
            m.textures.clear();
            Texture t{};
            t.id   = tex;
            t.type = "texture_diffuse";
            t.path = "__override__";
            m.textures.push_back(t);
        }
    }


    // Crea un Model a partir de un único Mesh construido en memoria
    Model(const std::vector<Vertex>& verts,
        const std::vector<unsigned int>& idx,
        const std::vector<Texture>& tex = {},
        bool gamma = false)
        : gammaCorrection(gamma)
    {
        meshes.emplace_back(verts, idx, tex);
    }

private:

    unsigned int overrideTexture = 0; 

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        std::filesystem::path p = std::filesystem::u8path(path);
        directory = p.parent_path().string();

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // --- datos base ---
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // vértices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex{};
            glm::vec3 v;

            // position
            v.x = mesh->mVertices[i].x;
            v.y = mesh->mVertices[i].y;
            v.z = mesh->mVertices[i].z;
            vertex.Position = v;

            // normal
            if (mesh->HasNormals()) {
                v.x = mesh->mNormals[i].x;
                v.y = mesh->mNormals[i].y;
                v.z = mesh->mNormals[i].z;
                vertex.Normal = v;
            }

            // texcoords / tangente / bitangente
            if (mesh->mTextureCoords[0]) {
                glm::vec2 t;
                t.x = mesh->mTextureCoords[0][i].x;
                t.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = t;

                v.x = mesh->mTangents[i].x;
                v.y = mesh->mTangents[i].y;
                v.z = mesh->mTangents[i].z;
                vertex.Tangent = v;

                v.x = mesh->mBitangents[i].x;
                v.y = mesh->mBitangents[i].y;
                v.z = mesh->mBitangents[i].z;
                vertex.Bitangent = v;
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // índices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // --- TEXTURAS ---
        // Si hay overrideTexture, ignoramos por completo los materiales del archivo
        if (overrideTexture != 0) {
            Texture t;
            t.id   = overrideTexture;
            t.type = "texture_diffuse";   // Mesh::Draw lo mapeará a "texture_diffuse1"
            t.path = "__override__";
            textures.push_back(t);
        } else {
            // Camino original: cargar desde materiales
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            std::vector<Texture> diffuseMaps  = loadMaterialTextures(material, aiTextureType_DIFFUSE,  "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            std::vector<Texture> normalMaps   = loadMaterialTextures(material, aiTextureType_HEIGHT,   "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

            std::vector<Texture> heightMaps   = loadMaterialTextures(material, aiTextureType_AMBIENT,  "texture_height");
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }

        // devolver la malla ya preparada
        return Mesh(vertices, indices, textures);
    }


    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char *path, const std::string &directory, bool /*gamma*/)
{
    std::filesystem::path texRel = std::filesystem::u8path(path);
    std::filesystem::path full = texRel.is_absolute()
        ? texRel
        : (directory.empty() ? texRel : std::filesystem::path(directory) / texRel);

    return LoadTextureDevIL(full.string().c_str(), /*genMipmaps=*/true);
}
#endif
