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
                            std::vector<glm::vec3>& m_tangents,
                            std::vector<glm::vec3>& m_bitangents)
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
        std::vector<glm::vec3> bitangents;
        

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

            // Tangents and bitangents are calculated by assimp
            aiVector3D tangent = mesh->mTangents[j];
            glm::vec3 tangentElem = glm::vec3(tangent.x, tangent.y, tangent.z);
            tangents.push_back(tangentElem);

            // If the mesh contains tangents, it automatically also contains bitangents.
            aiVector3D bitangent = mesh->mBitangents[j];
            glm::vec3 bitangentElem = glm::vec3(bitangent.x, bitangent.y, bitangent.z);
            bitangents.push_back(bitangentElem);
        }

        // Extract the textures from the mesh
        GLint diffuse = 0, metallic = 0, height = 0, normal = 0, smoothness = 0, ao = 0;

        // Get diffuse texture name of the mesh as main texture reference
        aiString textureName;
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);
        
        std::string texturePathCString(textureName.C_Str());
        std::string textureFileName = texturePathCString.substr(texturePathCString.find_last_of("/\\") + 1);
        std::string texturePath = m_texturePath + textureFileName;

        // Load the textures in parallel if any textures are asigned to the mesh
        if (textureFileName != "") {
            std::vector<std::string> texturePaths = std::vector<std::string>();

            // Get the texture IDs if the textures are already loaded; add them to the texturePaths vector for 
            // parallel loading if not
            diffuse = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "diffuseOriginal"));
            if (diffuse < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "diffuseOriginal"));
            smoothness = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "smoothness"));
            if (smoothness < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "smoothness"));
            height = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "height"));
            if (height < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "height"));
            ao = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "ao"));
            if (ao < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "ao"));
            metallic = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "metallic"));
            if (metallic < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "metallic"));
            normal = getTextureIDIfAlreadyLoaded(getTexturePathFromType(texturePath, "normal"));
            if (normal < 0)
                texturePaths.push_back(getTexturePathFromType(texturePath, "normal"));

            GLuint *textureHandles = loadTexturesInParallel(texturePaths);
            HANDLE_GL_ERRORS(":(");
            
            // Retrieve newly loaded textures from array
            int indexOffset = 0;
            if (diffuse < 0) {
                diffuse = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "diffuseOriginal"), diffuse));
                indexOffset++;
            }
            if (smoothness < 0) {
                smoothness = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "smoothness"), smoothness));
                indexOffset++;
            }
            if (height < 0) {
                height = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "height"), height));
                indexOffset++;
            }
            if (ao < 0) {
                ao = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "ao"), ao));
                indexOffset++;
            }
            if (metallic < 0) {
                metallic = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "metallic"), metallic));
                indexOffset++;
            }
            if (normal < 0) {
                normal = textureHandles[indexOffset];
                m_textureMap.insert(std::pair<std::string, GLuint>(getTexturePathFromType(texturePath, "normal"), normal));
                indexOffset++;
            }
        }

        GLuint textureID = 0;

        // Create a new mesh and add it to the list of meshes
        std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>(vertices, normals, uvs, indices, tangents, bitangents, color,
                                                               diffuse, smoothness, height, ao, metallic, normal);

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

GLuint Model::getTextureIDIfAlreadyLoaded(std::string path)
{
    GLuint textureID = 0;

    // Check if tex has already been loaded
    if (m_textureMap.find(path) != m_textureMap.end())
    {
        return m_textureMap[path];
    }   
    else
    {
        return -1;
    }
    
}

std::string Model::getTexturePathFromType(std::string diffusePath, std::string type) {
    // Determine file name from path
    int fileNameIndex = diffusePath.find_last_of("/");
    int extensionIndex = diffusePath.find_last_of(".") - fileNameIndex; 
    std::string materialName = diffusePath.substr(fileNameIndex + 1, extensionIndex - 1); 
    if (materialName.find("diffuseOriginal") != std::string::npos) {
        materialName = materialName.substr(0, materialName.find("diffuseOriginal") - 1);
    }

    if (materialName == "")
        materialName = "default";

    std::string folderPath = "./Assets/Special-Textures/" + materialName + "/";

    // Concatenate texture path from folder path and texture type
    std::string texturePath = folderPath + materialName + "_" + type + ".jpg";

    return texturePath;
}

Model::~Model()
{
    
}