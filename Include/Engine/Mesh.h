//
// Created by maxbr on 02.01.2023.
//

#pragma once

#include "Defs.h"
#include "Drawable.h"
#include <vector>
#include "ShaderProgram.h"



// Class that represents a mesh, extending the Drawable class, 
// additionally it has uv coordinates and the diffuse, specular, smoothness,
// height and ambient occulsion textures
class Mesh : public Drawable
{
public:
    Mesh(std::vector<glm::vec4> vertices,
         std::vector<glm::vec3> normals,
         std::vector<glm::vec2> uvs,
         std::vector<GLuint> indices,
         std::vector<glm::vec3> tangents,
         glm::vec4 diffuseColor,
         GLuint diffuseTexture,
         GLuint smoothnessTexture,
         GLuint heightTexture,
         GLuint ambientOcculsionTexture,
         GLuint metallicTexture,
         GLuint normal);

    void draw();

    // Setter for shader program
    void setShaderProgram(std::shared_ptr<ShaderProgram> shaderProgram);

    glm::vec3 getDiffuseColor();

private:
    // Shader handles
    std::shared_ptr<ShaderProgram> m_shaderProgram;

    // Texture handles
    GLuint m_diffuseTexture;
    GLuint m_smoothnessTexture;
    GLuint m_heightTexture;
    GLuint m_ambientOcculsionTexture;
    GLuint m_metallicTexture;
    GLuint m_normalTexture;
};
