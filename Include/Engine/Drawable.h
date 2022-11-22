//
// Created by maxbr on 10.04.2020.
//

#pragma once

#include "Defs.h"
#include <vector>

class Drawable
{
public:
    Drawable();

    void createBuffers();

    virtual void draw();

    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void setScale(glm::vec3 scale);

    glm::mat4 getModelMatrix();
protected:

    GLuint m_vao; //!< The OpenGL Vertex Array Object
    GLuint m_vertexbuffer; //!< A Vertex Buffer Object for storing vertex positions
    GLuint m_normalbuffer; //!< A Vertex Buffer Object for storing vertex normals
    GLuint m_uvbuffer; //!< A Vertex Buffer Object for storing vertex uv coordinates
    GLuint m_indexlist; //!< A Vertex Buffer Object for storing vertex indices
    GLuint m_tangentbuffer; //!< A Vertex Buffer Object for storing vertex tangents

    int m_numberOfPoints; //!< Number of all vertices
    int m_numberOfIndices; //!< Number of all indices

    std::vector<glm::vec4> m_vertices; //!< A list of all vertex positions
    std::vector<glm::vec3> m_normals; //!< A list of all vertex normals
    std::vector<glm::vec2> m_uvs; //!< A list of all vertex uv coordinates
    std::vector<unsigned int> m_index; //!< A list of all vertex indices
    std::vector<glm::vec3> m_tangents; //!< A list of all vertex tangents

private:
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation;
    glm::vec3 m_scale = glm::vec3(1.0f);
    glm::vec4 m_modelMatrix;
};
