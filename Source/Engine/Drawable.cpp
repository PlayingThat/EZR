//
// Created by maxbr on 10.04.2020.
//

#include "Engine/Drawable.h"
// Needed to resolve forward declaration
#include "Engine/Scene.h"

Drawable::Drawable(std::shared_ptr<Scene> scene,
                   glm::vec3 position, 
                   glm::vec3 rotation, 
                   glm::vec3 scale)
{
    m_scene = scene;
    m_basePosition = position;
    m_baseRotation = rotation;
    m_baseScale = scale;

    setupMemers();
};

Drawable::Drawable(glm::vec3 position, 
                   glm::vec3 rotation, 
                   glm::vec3 scale)
{
    m_basePosition = position;
    m_baseRotation = rotation;
    m_baseScale = scale;

    setupMemers();
}

void Drawable::setupMemers()
{
    m_baseRotationAngle = 0.0f;
 
    m_position = glm::vec3(0.0f);
    m_rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    m_scale = glm::vec3(1.0f);
}

Drawable::~Drawable()
{
}

void Drawable::draw()
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
    //glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_index.size()), GL_UNSIGNED_INT, 0);
}

void Drawable::createBuffers()
{
    m_numberOfPoints = m_vertices.size();
    m_numberOfIndices = m_indices.size();

    // create the buffers and bind the data
    if(m_numberOfPoints > 0)
    {
        glGenBuffers(1, &m_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_numberOfPoints * sizeof(glm::vec4), &m_vertices[0], GL_STATIC_DRAW);
    }

    if(m_normalbuffer == 0 && m_normals.size() > 0)
    {
        glGenBuffers(1, &m_normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_numberOfPoints * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    }

    if(m_uvbuffer == 0 && m_uvs.size() > 0)
    {
        glGenBuffers(1, &m_uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_numberOfPoints * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    }

    if(m_tangentbuffer == 0 && !m_tangents.empty())
    {
        if(m_tangents.empty())
        {
            //
        }
        glGenBuffers(1, &m_tangentbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_tangentbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_tangents.size() * sizeof(glm::vec3), &m_tangents[0], GL_STATIC_DRAW);
    }

    // Generate a buffer for the indices as well
    if(m_indexlist == 0 && !m_indices.empty())
    {
        glGenBuffers(1, &m_indexlist);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numberOfIndices * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
    }

    if(m_vao == 0)
        glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    if (m_normals.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if (m_uvs.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if (m_tangents.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_tangentbuffer);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);

    glBindVertexArray(m_vao);

}

void Drawable::setBasePosition(glm::vec3 position)
{
    m_basePosition = position;
}

void Drawable::setBaseRotation(float angle)
{
    m_baseRotationAngle = angle;
}

void Drawable::setBaseRotation(glm::vec3 rotation, float angle)
{
    m_baseRotation = rotation;
    m_baseRotationAngle = angle;
}

void Drawable::setBaseScale(glm::vec3 scale)
{
    m_baseScale = scale;
}

void Drawable::translate(glm::vec3 translation)
{
    m_position = translation;
}

void Drawable::rotate(float angle)
{
    m_rotationAngle = angle;
}

void Drawable::rotate(glm::vec3 rotation, float angle)
{
    m_rotation = rotation;
    m_rotationAngle = angle;
}

void Drawable::scale(glm::vec3 scale)
{
    m_scale = scale;
}

glm::mat4 Drawable::getBaseModelMatrix()
{
    return glm::translate(
                glm::rotate(
                    glm::scale(glm::mat4(1.0f), this->m_baseScale), 0.01745f * m_baseRotationAngle, this->m_baseRotation), this->m_basePosition);
}

glm::mat4 Drawable::getModelMatrix()
{
    return glm::translate(
                glm::rotate(
                    glm::scale(getBaseModelMatrix(), this->m_scale), 0.01745f * m_rotationAngle, this->m_rotation), this->m_position);
}

