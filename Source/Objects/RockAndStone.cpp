//
// Created by maxbr on 29.12.2022.
// 
// Using dwarf model from Model by thecubber, commissioned by the http://opengameart.org/ community
// Modified by Beast@opengameart
//

#include "Objects/RockAndStone.h"

RockAndStone::RockAndStone(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    create();

    // Shader setup
    m_basicVertexShader = std::make_shared<Shader>("./Assets/Shader/Basic.vert");
    m_basicFragmentShader = std::make_shared<Shader>("./Assets/Shader/Basic.frag");

    m_basicShaderProgram = std::make_unique<ShaderProgram>("Basic");
    m_basicShaderProgram->addShader(m_basicVertexShader);
    m_basicShaderProgram->addShader(m_basicFragmentShader);

    m_basicShaderProgram->link();

    Drawable::setPosition(glm::vec3(0.2f, -0.2f, -1.2f));
}

void RockAndStone::create()
{
    m_modelLoader->loadModel("Assets/Models/rock_and_stone.obj", m_vertices, m_normals, m_uvs, m_index, m_tangents);
    createBuffers();

    // Silence is golden
}

void RockAndStone::draw()
{
    m_basicShaderProgram->setMat4("projectionMatrix", *m_scene->getState()->getCamera()->getProjectionMatrix());
    m_basicShaderProgram->setMat4("viewMatrix", *m_scene->getState()->getCamera()->getViewMatrix());
    m_basicShaderProgram->setMat4("modelMatrix", Drawable::getModelMatrix());

    Drawable::draw();
}
