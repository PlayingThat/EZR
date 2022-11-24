//
// Created by maxbr on 21.11.2022.
//

#include "Defs.h"
#include <vector>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

class ModelLoader
{
public:
    ModelLoader();
    ~ModelLoader();

    /**
     * @brief Loads a model from a file. Any of the formats supported by Assimp can be loaded. 
     * See https://github.com/assimp/assimp/blob/master/doc/Fileformats.md for more imformation.
     * 
     * @param path The path to the model file. Keep relative to the build folder, i.e. './Assets/Models/example.fbx'.
     * @return true On sucessful load.
     * @return false On failed load. See log for details.
     */
    bool loadModel(std::string path);

    std::vector<glm::vec4> getVertices();
    std::vector<glm::vec3> getNormals();
    std::vector<glm::vec2> getUVs();
    std::vector<unsigned int> getIndices();
    std::vector<glm::vec3> getTangents();

    int getNumberOfPoints();
    int getNumberOfIndices();

private:
    std::vector<glm::vec4> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<unsigned int> m_index;
    std::vector<glm::vec3> m_tangents;

    int m_numberOfPoints; //@brief Number of points in the mesh
    int m_numberOfIndices; //@brief Number of indices in the mesh
};