//
// Created by maxbr on 21.11.2022.
//

#include "Engine/ModelLoader.h"

// Resolve forward declaration
#include "Engine/Mesh.h"

ModelLoader::ModelLoader(std::shared_ptr<Scene> scene)
{
    m_scene = scene;
}

bool ModelLoader::loadModel(std::string path,
                            std::vector<glm::vec4>& m_vertices,
                            std::vector<glm::vec3>& m_normals,
                            std::vector<glm::vec2>& m_uvs,
                            std::vector<unsigned int>& m_indices,
                            std::vector<glm::vec3>& m_tangents)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR(importer.GetErrorString());
        return false;
    }

    // Iterate over all meshes in the model
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        
        aiMaterial* test = scene->mMaterials[mesh->mMaterialIndex];    
        // loop over textures count
        for (unsigned int j = 0; j < test->GetTextureCount(aiTextureType_DIFFUSE); j++)
        {
            aiString str;

            scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            std::string texturePath = str.C_Str();
            printf("Texture path");
        }
        
        
        // Extract indices from the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                m_indices.push_back(face.mIndices[k]);
            }
        }

        // Extract the vertices, normals, and textures from the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            
            // Vertices
            aiVector3D vertex = mesh->mVertices[j];
            glm::vec4 vertexElem = glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
            m_vertices.push_back(vertexElem);
            
            // Normals
            aiVector3D normal = mesh->mNormals[j];
            glm::vec3 normalElem = glm::vec3(normal.x, normal.y, normal.z);
            m_normals.push_back(normalElem);
            
            // Textures
            if (mesh->mTextureCoords[0])
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                glm::vec2 texElem = glm::vec2(texCoord.x, texCoord.y);
                m_uvs.push_back(texElem);
            }
            else
            {
                // If the mesh does not have texture coordinates, use a default value of (0,0)
                m_uvs.push_back(glm::vec2(0.0f, 0.0f));
            }

            // Tangents (what if they don't exist?)
            aiVector3D tangent = mesh->mTangents[j];
            glm::vec3 tangentElem = glm::vec3(tangent.x, tangent.y, tangent.z);
            m_tangents.push_back(tangentElem);
            // If the mesh contains tangents, it automatically also contains bitangents.
        }

        GLuint textureID = 0;
        // Create a new mesh and add it to the list of meshes
        std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>(m_vertices, m_normals, m_uvs, m_indices, m_tangents,
        textureID, textureID, textureID, textureID, textureID);
    }
    return true;
}

void ModelLoader::loadTextures(std::string path)
{
    
}

ModelLoader::~ModelLoader()
{
    
}