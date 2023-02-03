//
// Created by jesbu on 03.02.2023
// 


#include "Objects/Stone2.h"

Stone2::Stone2(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_model = std::make_unique<Model>(m_scene);
    create();
}

void Stone2::create()
{
    // Load model
    m_model->loadModel("./Assets/Models/Stone2.fbx", m_vertices, m_normals, m_uvs, m_indices, m_tangents, m_bitangents);
    createBuffers();

    Drawable::setBaseRotation(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);

    // Silence is golden
}

void Stone2::draw()
{
    Drawable::rotate(glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);

    m_model->draw();
}
