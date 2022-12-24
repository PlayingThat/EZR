//
// Created by maxbr on 21.11.2022.
//

#include "Objects/ColorfullTriangle.h"

ColorfullTriangle::ColorfullTriangle()
{
    m_triangle = std::make_shared<Triangle>();

    // Shader setup
    m_basicVertexShader = std::make_shared<Shader>("./Assets/Shader/Basic.vert");
    m_basicFragmentShader = std::make_shared<Shader>("./Assets/Shader/Basic.frag");

    m_basicShaderProgram = std::make_unique<ShaderProgram>("Basic");
    m_basicShaderProgram->addShader(m_basicVertexShader);
    m_basicShaderProgram->addShader(m_basicFragmentShader);

    m_basicShaderProgram->link();

    // Triangle color
    m_color =  std::make_unique<float[]>(3); 
    m_color.get()[1] = 1;
}

ColorfullTriangle::~ColorfullTriangle()
{

}

void ColorfullTriangle::draw()
{
    // GUI
    ImGui::Begin("Triangle");
    ImGui::ColorPicker3("Triangle Color", m_color.get());
    ImGui::End();

    // Draw triangle
    m_basicShaderProgram->use();
    m_basicShaderProgram->setVec3("color", glm::make_vec3(m_color.get()));

    // Keep in screen space 
    m_basicShaderProgram->setMat4("projectionMatrix", glm::mat4(1.0f));
    m_basicShaderProgram->setMat4("viewMatrix", glm::mat4(1.0f));
    m_basicShaderProgram->setMat4("modelMatrix", glm::mat4(1.0f));
    m_triangle->draw();
    m_basicShaderProgram->setMat4("modelMatrix", glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 1.0f, 0.0f)));
    m_basicShaderProgram->setMat4("modelMatrix", glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 2.0f, 0.0f)));
    m_basicShaderProgram->setVec3("color", glm::vec3(1.0f, 0.5f, 0.0f));
    m_triangle->draw();
}

