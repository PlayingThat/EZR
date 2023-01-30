//
// Created by maxbr on 21.11.2022.
//

#include "Defs.h"
#include "Texture.h"
#include <vector>
#include <map>
#include <omp.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

class Mesh;
class Scene;

class Model
{
public:
    Model(std::shared_ptr<Scene> scene);
    ~Model();

    /**
     * @brief Loads a model from a file. Any of the formats supported by Assimp can be loaded. 
     * See https://github.com/assimp/assimp/blob/master/doc/Fileformats.md for more imformation.
     * 
     * @param path The path to the model file. Keep relative to the build folder, i.e. './Assets/Models/example.fbx'.
     * @return true On sucessful load.
     * @return false On failed load. See log for details.
     */
    bool loadModel(std::string path,
                    std::vector<glm::vec4>& m_vertices,
                    std::vector<glm::vec3>& m_normals,
                    std::vector<glm::vec2>& m_uvs,
                    std::vector<unsigned int>& m_indices,
                    std::vector<glm::vec3>& m_tangents,
                    std::vector<glm::vec3>& m_bitangents);

    GLuint loadTexture(std::string path, std::string typeString);

    void draw();

private:
    // Load texture from provided path and type
    GLuint loadTextureFromType(std::string diffusePath, std::string type);
    
    std::shared_ptr<Scene> m_scene;
    std::vector<std::shared_ptr<Mesh>> m_meshes;

    // Path to main texture folder
    std::string m_texturePath;

    // Map between texture names and texture IDs
    std::map<std::string, GLuint> m_textureMap;
};
