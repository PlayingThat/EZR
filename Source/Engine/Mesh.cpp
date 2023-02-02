//
// Created by maxbr on 02.01.2023.
//

#include "Engine/Mesh.h"

Mesh::Mesh(std::vector<glm::vec4> vertices,
           std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs,
           std::vector<GLuint> indices,
           std::vector<glm::vec3> tangents,
           std::vector<glm::vec3> bitangents,
           glm::vec4 diffuseColor,
           GLuint diffuseTexture,
           GLuint smoothnessTexture,
           GLuint heightTexture,
           GLuint ambientOcculsionTexture,
           GLuint metallicTexture,
           GLuint normalTexture) : Drawable()
{
    // Set geometry information
    m_vertices = vertices;
    m_normals = normals;
    m_uvs = uvs;
    m_indices = indices;
    m_tangents = tangents;
    m_bitangents = bitangents;

    // Set material color
    m_diffuseColor = diffuseColor;

    Drawable::createBuffers();

    // Set texture information
    m_diffuseTexture = diffuseTexture;
    m_smoothnessTexture = smoothnessTexture;
    m_heightTexture = heightTexture;
    m_ambientOcculsionTexture = ambientOcculsionTexture;
    m_metallicTexture = metallicTexture;
    m_normalTexture = normalTexture;
}

void Mesh::draw()
{
    // Get active shader program and set the diffuse color
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    glUseProgram(prog);

    // Bind the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
    glUniform1i(glGetUniformLocation(prog, "diffuseSampler"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_metallicTexture);
    glUniform1i(glGetUniformLocation(prog, "metalSampler"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_smoothnessTexture);
    glUniform1i(glGetUniformLocation(prog, "smoothnessSampler"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_heightTexture);
    glUniform1i(glGetUniformLocation(prog, "heightSampler"), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glUniform1i(glGetUniformLocation(prog, "normalSampler"), 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_ambientOcculsionTexture);
    glUniform1i(glGetUniformLocation(prog, "ambientOcclusionSampler"), 5);


    GLuint uniformLocation = glGetUniformLocation(prog, "DiffuseColor");
    glUniform4fv(uniformLocation, 1, glm::value_ptr(m_diffuseColor));

    // Transparency
    uniformLocation = glGetUniformLocation(prog, "alpha");
    glUniform1f(uniformLocation, 0.5f);  // TODO: Make this a property of something, mybe the material?
    
    Drawable::draw();

}

void Mesh::setShaderProgram(std::shared_ptr<ShaderProgram> shaderProgram)
{
    m_shaderProgram = shaderProgram;
}
