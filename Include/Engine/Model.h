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
                    std::vector<glm::vec3>& m_tangents);

    GLuint loadTexture(aiTextureType type, std::string path, std::string typeString);

    void draw();

private:
    std::shared_ptr<Scene> m_scene;
    std::vector<std::shared_ptr<Mesh>> m_meshes;

    // Path to main texture folder
    std::string m_texturePath;

    // Map between texture names and texture IDs
    std::map<std::string, GLuint> m_textureMap;

    // Map between diffuse texture path of meshes and a map of their special texture types and paths
    // aiTextureType_DIFFUSE is used as a general index for the mesh's textures
    // The texture type also specifies the texture's usage, e.g. aiTextureType_DIFFUSE for diffuse textures
    // aiTextureType_DIFFUSE_ROUGHNESS is used for smoothness textures
    std::map<std::string, std::map<aiTextureType, std::string>> m_objectToTextureMap = {
        {"./Assets/Relevant-Textures/Ghost.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Ghost.png"}}},
        {"./Assets/Relevant-Textures/Cylinder.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cylinder.png"}}},
        {"./Assets/Relevant-Textures/Cylinder-Band.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cylinder-Band.png"}}},
        {"./Assets/Relevant-Textures/Gingerbread.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gingerbread.png"},
                                                       {aiTextureType_AMBIENT_OCCLUSION, "./Assets/Special-Textures/Gingerbread/Gingerbread_ao.png"},
                                                       {aiTextureType_NORMALS, "./Assets/Special-Textures/Gingerbread/Gingerbread_normal.png"},
                                                       {aiTextureType_HEIGHT, "./Assets/Special-Textures/Gingerbread/Gingerbread_height.png"},
                                                       {aiTextureType_DIFFUSE_ROUGHNESS, "./Assets/Special-Textures/Gingerbread/Gingerbread_smoothness.png"}}},

        {"./Assets/Relevant-Textures/Icing.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Icing.png"}}},
        {"./Assets/Relevant-Textures/Gummidrop-green.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gummidrop-green.png"}}},
        {"./Assets/Relevant-Textures/Gummidrop-pink.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gummidrop-pink.png"}}},
        {"./Assets/Relevant-Textures/Brezel.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Brezel.png"}}},
        {"./Assets/Relevant-Textures/Smarties-yellow.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-yellow.png"}}},
        {"./Assets/Relevant-Textures/Smarties-red.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-red.png"}}},
        {"./Assets/Relevant-Textures/Smarties-green.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-green.png"}}},
        {"./Assets/Relevant-Textures/Smarties-lila.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-lila.png"}}},
        {"./Assets/Relevant-Textures/Cookie-light.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cookie-light.png"}}},
        {"./Assets/Relevant-Textures/Cookie-dark.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cookie-dark.png"}}},
        {"./Assets/Relevant-Textures/Snow.jpg", {{aiTextureType_DIFFUSE, "/Assets/Relevant-Textures/Snow.jpg"}}},
        {"./Assets/Relevant-Textures/Branches.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Branches.jpg"}}},
        {"./Assets/Relevant-Textures/Carrot.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Carrot.jpg"}}},
        {"./Assets/Relevant-Textures/Metal.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Metal.png"}}},
        {"./Assets/Relevant-Textures/Scarf.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Scarf.png"}}},
        {"./Assets/Relevant-Textures/Stone.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Stone.jpg"}}},
        {"./Assets/Relevant-Textures/Tree.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Tree.jpg"}}},
        {"./Assets/Relevant-Textures/LeavesD.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/LeavesD.png"}}}
    };
};
