//
// Created by maxbr on 29.12.2022.
// 
// Using dwarf model from Model by thecubber, commissioned by the http://opengameart.org/ community
// Modified by Beast@opengameart
//

#include "Objects/Ghost.h"

Ghost::Ghost(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    create();

    // Shader setup
    m_basicVertexShader = std::make_shared<Shader>("./Assets/Shader/Basic.vert");
    m_textureFragmentShader = std::make_shared<Shader>("./Assets/Shader/Basic.frag");

    m_basicShaderProgram = std::make_unique<ShaderProgram>("Basic");
    m_basicShaderProgram->addShader(m_basicVertexShader);
    m_basicShaderProgram->addShader(m_textureFragmentShader);
    printf("Linking shader program...\n");

    m_basicShaderProgram->link();
}

void Ghost::create()
{
    // Load model
    m_modelLoader->loadModel("Assets/Models/Ghost.fbx", m_vertices, m_normals, m_uvs, m_index, m_tangents);
    createBuffers();

    // Load texture
    

    // Silence is golden
}

void Ghost::draw()
{
    Drawable::setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);

    m_basicShaderProgram->setMat4("projectionMatrix", *m_scene->getState()->getCamera()->getProjectionMatrix());
    m_basicShaderProgram->setMat4("viewMatrix", *m_scene->getState()->getCamera()->getViewMatrix());
    m_basicShaderProgram->setMat4("modelMatrix", Drawable::getModelMatrix());

    Drawable::draw();
}
