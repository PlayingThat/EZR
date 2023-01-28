//
// Created by maxbr on 21.11.2022.
//

#include "Engine/Model.h"

// Resolve forward declaration
#include "Engine/Mesh.h"

Model::Model(std::shared_ptr<Scene> scene)
{
    m_scene = scene;

    m_meshes = std::vector<std::shared_ptr<Mesh>>();

    m_texturePath = "./Assets/Special-Textures/";

    m_textureMap = std::map<std::string, GLuint>();
}

bool Model::loadModel(std::string path,
                            std::vector<glm::vec4>& m_vertices,
                            std::vector<glm::vec3>& m_normals,
                            std::vector<glm::vec2>& m_uvs,
                            std::vector<unsigned int>& m_indices,
                            std::vector<glm::vec3>& m_tangents)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_GenSmoothNormals | 
                                                    aiProcess_CalcTangentSpace | 
                                                    aiProcess_Triangulate | 
                                                    aiProcess_ConvertToLeftHanded);

    LOG_INFO("Try to load model " + path);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR(importer.GetErrorString());
        return false;
    }

    // Iterate over all meshes in the model
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        std::vector<glm::vec4> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs; 
        std::vector<unsigned int> indices; 
        std::vector<glm::vec3> tangents;
        

        aiMesh* mesh = scene->mMeshes[i];
        
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex]; 

        // Extract color from material
        aiColor4D dif;
        glm::vec4 color;
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &dif))
        {
            color = glm::vec4(dif.r, dif.g, dif.b, dif.a);
        }
        else
        {
            color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
   
        
        // Extract indices from the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }

        // Extract the vertices, normals, and textures from the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            
            // Vertices
            aiVector3D vertex = mesh->mVertices[j];
            glm::vec4 vertexElem = glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
            vertices.push_back(vertexElem);
            
            // Normals
            aiVector3D normal = mesh->mNormals[j];
            glm::vec3 normalElem = glm::vec3(normal.x, normal.y, normal.z);
            normals.push_back(normalElem);
            
            // Textures
            if (mesh->mTextureCoords[0])
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                glm::vec2 texElem = glm::vec2(texCoord.x, texCoord.y);
                uvs.push_back(texElem);
            }
            else
            {
                // If the mesh does not have texture coordinates, use a default value of (0,0)
                uvs.push_back(glm::vec2(0.0f, 0.0f));
            }

            // Tangents (what if they don't exist?)
            aiVector3D tangent = mesh->mTangents[j];
            glm::vec3 tangentElem = glm::vec3(tangent.x, tangent.y, tangent.z);
            tangents.push_back(tangentElem);
            // If the mesh contains tangents, it automatically also contains bitangents.
        }

        // Extract the textures from the mesh
        GLuint diffuse = 0, metal = 0, height = 0, normal = 0, smoothness = 0, ao = 0;

        // Get diffuse texture name of the mesh as main texture reference
        aiString textureName;
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);
        
        std::string texturePathCString(textureName.C_Str());
        std::string textureFileName = texturePathCString.substr(texturePathCString.find_last_of("/\\") + 1);
        std::string texturePath = m_texturePath + textureFileName;

        // Check if diffuse texture is present in object texture map
        if (m_objectToTextureMap.find(texturePath) != m_objectToTextureMap.end())
        {
            // Load textures using helper function loadTextureTypeIfPossible
            diffuse = loadTexture(aiTextureType_DIFFUSE, texturePath, "diffuse");
            metal = loadTexture(aiTextureType_METALNESS, texturePath, "metal");
            height = loadTexture(aiTextureType_HEIGHT, texturePath, "height");
            normal = loadTexture(aiTextureType_NORMALS, texturePath, "normal");
            smoothness = loadTexture(aiTextureType_DIFFUSE_ROUGHNESS, texturePath, "smoothness");
            ao = loadTexture(aiTextureType_AMBIENT_OCCLUSION, texturePath, "ambient occlusion");
        }

        GLuint textureID = 0;
        // Create a new mesh and add it to the list of meshes
        std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>(vertices, normals, uvs, indices, tangents, color,
                                                               diffuse, smoothness, height, ao, metal, normal);

        m_meshes.push_back(newMesh);
    }
    return true;
}

void Model::draw()
{
    for (std::shared_ptr<Mesh> mesh : m_meshes)
    {
        mesh->draw();
    }
}

GLuint Model::loadTexture(aiTextureType type, std::string path, std::string typeString)
{
    std::string finalTexturePath;
    if (m_objectToTextureMap.at(path).find(type) != m_objectToTextureMap.at(path).end())
    {
        finalTexturePath = m_objectToTextureMap.at(path).at(type);
    }
    else
    {
        finalTexturePath = "./Assets/Special-Textures/black.png";
    }

    GLuint textureID = 0;
    // Load texture from file

    // Check if tex has already been loaded
    if (m_textureMap.find(finalTexturePath) != m_textureMap.end())
    {
        textureID = m_textureMap[finalTexturePath];
    }   
    else
    {
        LOG_INFO("Loading " + typeString + " texture: " + finalTexturePath);
        textureID = createTextureFromFile(finalTexturePath);
        m_textureMap[finalTexturePath] = textureID;
    }

    return textureID;
    
}

Model::~Model()
{
    
}