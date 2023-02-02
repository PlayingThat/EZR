//
// Created by maxbr on 29.12.2022.
// 
// Using dwarf model from Model by thecubber, commissioned by the http://opengameart.org/ community
// Modified by Beast@opengameart
//

#include "Objects/Ghost.h"

Ghost::Ghost(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_model = std::make_unique<Model>(m_scene);
    create();
}

void Ghost::create()
{
    // Load model
    m_model->loadModel("./Assets/Models/Ghost.fbx", m_vertices, m_normals, m_uvs, m_indices, m_tangents, m_bitangents);
    createBuffers();

    Drawable::setBaseRotation(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);

    // Silence is golden
}

void Ghost::draw()
{
    Drawable::rotate(glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);

    m_model->draw();
}
