//
// Created by maxbr on 10.04.2020.
//

#pragma once

#include "Defs.h"
#include <vector>

// Forward declaration
class Scene;

class Drawable
{
public:
    Drawable(std::shared_ptr<Scene> scene,
             glm::vec3 position = glm::vec3(0.0f), 
             glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f), 
             glm::vec3 scale = glm::vec3(1.0f));
    // Constructor without scene for meshes that won't be drawn without a parent object
    Drawable(glm::vec3 position = glm::vec3(0.0f), 
             glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f), 
             glm::vec3 scale = glm::vec3(1.0f));
    ~Drawable();

    void createBuffers();

    virtual void draw();

    // For initial setup of the object i.e. to make sure the object is upright
    void setBasePosition(glm::vec3 position);
    void setBaseRotation(float angle);
    void setBaseRotation(glm::vec3 rotation, float angle);
    void setBaseScale(glm::vec3 scale);

    // For moving the object around
    void translate(glm::vec3 translation);
    void rotate(glm::vec3 rotation, float angle);
    void rotate(float angle);
    void scale(glm::vec3 scale);

    glm::mat4 getBaseModelMatrix();
    glm::mat4 getModelMatrix();
protected:

    GLuint m_vao = 0;
    GLuint m_vertexbuffer = 0;
    GLuint m_normalbuffer = 0;
    GLuint m_uvbuffer = 0;
    GLuint m_indexlist = 0;
    GLuint m_tangentbuffer = 0;

    int m_numberOfPoints; // @brief Number of points in the mesh
    int m_numberOfIndices; // @brief Number of indices in the mesh

    std::vector<glm::vec4> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs; 
    std::vector<unsigned int> m_indices; 
    std::vector<glm::vec3> m_tangents;

    std::shared_ptr<Scene> m_scene;

    glm::vec4 m_diffuseColor;

private:
    void setupMemers();

    glm::vec3 m_basePosition; 
    glm::vec3 m_baseRotation;
    float m_baseRotationAngle;
    glm::vec3 m_baseScale;
    glm::vec4 m_modelMatrix;

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    float m_rotationAngle;
    glm::vec3 m_scale;


};
