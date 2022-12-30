//
// Created by maxbr on 25.05.2020.
//

#include "Objects/Quad.h"

Quad::Quad(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    create();
}

void Quad::create()
{
    m_vertices.push_back(glm::vec4(-1.0f, -1.0f, 0.0, 1.0));
    m_vertices.push_back(glm::vec4(1.0f, -1.0f, 0.0, 1.0));
    m_vertices.push_back(glm::vec4(-1.0f, 1.0f, 0.0, 1.0));
    m_vertices.push_back(glm::vec4(1.0f, 1.0f, 0.0, 1.0));
    m_vertices.push_back(glm::vec4(-1.0f, 1.0f, 0.0, 1.0));
    m_vertices.push_back(glm::vec4(1.0f, -1.0f, 0.0, 1.0));

    createBuffers();
}
