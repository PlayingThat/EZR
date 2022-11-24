//
// Created by maxbr on 21.11.2022.
//

#include "Engine/ModelLoader.h"

ModelLoader::ModelLoader()
{
    m_vertices = std::vector<glm::vec4>();
    m_normals = std::vector<glm::vec3>();
    m_uvs = std::vector<glm::vec2>();
    m_index = std::vector<unsigned int>();
    m_tangents = std::vector<glm::vec3>();
}

bool ModelLoader::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    return true;
}

ModelLoader::~ModelLoader()
{
    m_vertices.clear();
    m_normals.clear();
    m_uvs.clear();
    m_index.clear();
    m_tangents.clear();
}