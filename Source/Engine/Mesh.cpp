//
// Created by maxbr on 02.01.2023.
//

#include "Engine/Mesh.h"

Mesh::Mesh(std::vector<glm::vec4> vertices,
           std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs,
           std::vector<GLuint> indices,
           std::vector<glm::vec3> tangents,
           GLuint diffuseTexture,
           GLuint specularTexture,
           GLuint smoothnessTexture,
           GLuint heightTexture,
           GLuint ambientOcculsionTexture) : Drawable()
{
    // Set geometry information
    m_vertices = vertices;
    m_normals = normals;
    m_uvs = uvs;
    m_indices = indices;
    m_tangents = tangents;

    Drawable::createBuffers();

    // Set texture information
    m_diffuseTexture = diffuseTexture;
    m_specularTexture = specularTexture;
    m_smoothnessTexture = smoothnessTexture;
    m_heightTexture = heightTexture;
    m_ambientOcculsionTexture = ambientOcculsionTexture;
}

void Mesh::draw()
{
    // Bind the shader program
    //m_shaderProgram->use();

    // // Bind the textures
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, m_specularTexture);
    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_2D, m_smoothnessTexture);
    // glActiveTexture(GL_TEXTURE3);
    // glBindTexture(GL_TEXTURE_2D, m_heightTexture);
    // glActiveTexture(GL_TEXTURE4);
    // glBindTexture(GL_TEXTURE_2D, m_ambientOcculsionTexture);

    Drawable::draw();

}

void Mesh::setShaderProgram(std::shared_ptr<ShaderProgram> shaderProgram)
{
    m_shaderProgram = shaderProgram;
}