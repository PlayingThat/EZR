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

    /* Max Idee
    std::string diffuseToTypePath(std::string diffusePath, std::string type) {
        std::string folderPath = diffusePath.split; // das sind wahrscheinlich 2-3 zeilen
        std::string baseFileNameName = diffusePath.split("/")[1].split("."); // dateinamen holen
        return folderPath + baseFileName + type + ".png";
    } 
    */ 

    //Jessys Idee 1
    std::string loadTexturePath (std::string materialName, int type) {

        std::string folderPath = "./Assets/Special-Textures/" + materialName + "/";

        std::string pathDiffuse = folderPath + materialName + "_diffuseOriginal.jpg";
        std::string pathAO = folderPath + materialName + "_ao.jpg";
        std::string pathNormal = folderPath + materialName + "_normal.jpg";
        std::string pathHeight = folderPath + materialName + "_height.jpg";
        std::string pathRoughness = folderPath + materialName + "_smoothness.jpg";
        std::string pathMetallic = folderPath + materialName + "_metallic.jpg";

        std::string texturePath;

        if (type == 0) {texturePath = pathDiffuse;}
        else if (type == 1) {texturePath = pathAO;}
        else if (type == 2) {texturePath = pathNormal;}
        else if (type == 3) {texturePath = pathHeight;}
        else if (type == 4) {texturePath = pathRoughness;}
        else if (type == 5) {texturePath = pathMetallic;}
        else LOG_ERROR ("type not defined");
    
        return texturePath;
    }

    /* Jessys Idee 2
    std::map<std::string, std::map<aiTextureType, std::string>> loadTexturePaths (std::string materialName) {

        std::string folderPath = "./Assets/Special-Textures/" + materialName + "/";

        std::string pathDiffuse = folderPath + materialName + "_diffuseOriginal.jpg";
        std::string pathAO = folderPath + materialName + "_ao.jpg";
        std::string pathNormal = folderPath + materialName + "_normal.jpg";
        std::string pathHeight = folderPath + materialName + "_height.jpg";
        std::string pathRoughness = folderPath + materialName + "_smoothness.jpg";
        std::string pathMetallic = folderPath + materialName + "_metallic.jpg";

        std::map<std::string, std::map<aiTextureType, std::string>> objectToTexture = {
        pathDiffuse, {{aiTextureType_DIFFUSE, pathDiffuse},
                    {aiTextureType_AMBIENT_OCCLUSION, pathAO},
                    {aiTextureType_NORMALS, pathNormal},
                    {aiTextureType_HEIGHT, pathHeight},
                    {aiTextureType_DIFFUSE_ROUGHNESS, pathRoughness}}
         };

         return objectToTexture; 
    } 
    */  

    // Map between diffuse texture path of meshes and a map of their special texture types and paths
    // aiTextureType_DIFFUSE is used as a general index for the mesh's textures
    // The texture type also specifies the texture's usage, e.g. aiTextureType_DIFFUSE for diffuse textures
    // aiTextureType_DIFFUSE_ROUGHNESS is used for smoothness textures
    std::map<std::string, std::map<aiTextureType, std::string>> m_objectToTextureMap = {
        //{"./Assets/Relevant-Textures/Ghost.png", {{aiTextureType_DIFFUSE, "./Assets/Special-Textures/Ghost.png"}}},
        // Idee 1
        {loadTexturePath("Ghost", 0), {{aiTextureType_DIFFUSE, loadTexturePath("Ghost", 0)},
                                        {aiTextureType_AMBIENT_OCCLUSION, loadTexturePath("Ghost", 1)},
                                        {aiTextureType_NORMALS, loadTexturePath("Ghost", 2)},
                                        {aiTextureType_HEIGHT, loadTexturePath("Ghost", 3)},
                                        {aiTextureType_DIFFUSE_ROUGHNESS, loadTexturePath("Ghost", 4)}}},
        /* Idee 2
        {loadTexturePaths("Ghost")},  */
                                        
        {"./Assets/Relevant-Textures/Cylinder.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cylinder.png"}}},
        {"./Assets/Relevant-Textures/Cylinder-Band.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cylinder-Band.png"}}},
        {"./Assets/Relevant-Textures/Gingerbread.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gingerbread.png"},
                                                       {aiTextureType_AMBIENT_OCCLUSION, "./Assets/Special-Textures/Gingerbread/Gingerbread_ao.jpg"},
                                                       {aiTextureType_NORMALS, "./Assets/Special-Textures/Gingerbread/Gingerbread_normal.jpg"},
                                                       {aiTextureType_HEIGHT, "./Assets/Special-Textures/Gingerbread/Gingerbread_height.jpg"},
                                                       {aiTextureType_DIFFUSE_ROUGHNESS, "./Assets/Special-Textures/Gingerbread/Gingerbread_smoothness.jpg"}}},

        {"./Assets/Relevant-Textures/Icing.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Icing.png"}}},
        {"./Assets/Relevant-Textures/Gummidrop-green.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gummidrop-green.png"},
                                                       {aiTextureType_AMBIENT_OCCLUSION, "./Assets/Special-Textures/Gummidrop/Gummidrop_ao.jpg"},
                                                       {aiTextureType_NORMALS, "./Assets/Special-Textures/Gummidrop/Gummidrop_normal.jpg"},
                                                       {aiTextureType_HEIGHT, "./Assets/Special-Textures/Gummidrop/Gummidrop_height.jpg"},
                                                       {aiTextureType_DIFFUSE_ROUGHNESS, "./Assets/Special-Textures/Gummidrop/Gummidrop_smoothness.jpg"}}},
                                                       
        {"./Assets/Relevant-Textures/Gummidrop-pink.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Gummidrop-pink.png"},
                                                       {aiTextureType_AMBIENT_OCCLUSION, "./Assets/Special-Textures/Gummidrop/Gummidrop_ao.jpg"},
                                                       {aiTextureType_NORMALS, "./Assets/Special-Textures/Gummidrop/Gummidrop_normal.jpg"},
                                                       {aiTextureType_HEIGHT, "./Assets/Special-Textures/Gummidrop/Gummidrop_height.jpg"},
                                                       {aiTextureType_DIFFUSE_ROUGHNESS, "./Assets/Special-Textures/Gummidrop/Gummidrop_smoothness.jpg"}}},

        {"./Assets/Relevant-Textures/Brezel.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Brezel.png"}}},
        {"./Assets/Relevant-Textures/Smarties-yellow.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-yellow.png"}}},
        {"./Assets/Relevant-Textures/Smarties-red.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-red.png"}}},
        {"./Assets/Relevant-Textures/Smarties-green.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-green.png"}}},
        {"./Assets/Relevant-Textures/Smarties-lila.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Smarties-lila.png"}}},
        {"./Assets/Relevant-Textures/Cookie-light.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cookie-light.png"}}},
        {"./Assets/Relevant-Textures/Cookie-dark.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Cookie-dark.png"}}},
        {"./Assets/Relevant-Textures/Snow.jpg", {{aiTextureType_DIFFUSE, "/Assets/Relevant-Textures/Snow.jpg"}}},
        /*{loadTexturePath("Branches", 0), {{aiTextureType_DIFFUSE, loadTexturePath("Branches", 0)},
                                        {aiTextureType_AMBIENT_OCCLUSION, loadTexturePath("Branches", 1)},
                                        {aiTextureType_NORMALS, loadTexturePath("Branches", 2)},
                                        {aiTextureType_HEIGHT, loadTexturePath("Branches", 3)},
                                        {aiTextureType_DIFFUSE_ROUGHNESS, loadTexturePath("Branches", 4)}}},
                                        */
        {"./Assets/Relevant-Textures/Carrot.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Carrot.jpg"}}},
        {"./Assets/Relevant-Textures/Metal.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Metal.png"}}},
        {"./Assets/Relevant-Textures/Scarf.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Scarf.png"}}},
        {"./Assets/Relevant-Textures/Stone.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Stone.jpg"}}},
        {"./Assets/Relevant-Textures/Tree.jpg", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/Tree.jpg"}}},
        {"./Assets/Relevant-Textures/LeavesD.png", {{aiTextureType_DIFFUSE, "./Assets/Relevant-Textures/LeavesD.png"}}}
    };
};
