//
// Created by maxbr on 25.05.2020.
//

#include "Objects/ScreenFillingQuad.h"

ScreenFillingQuad::ScreenFillingQuad(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_scene = scene;
    m_quad = std::make_shared<Quad>(m_scene);

    m_fragmentShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.frag");
    m_drawFBOShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.vert");
    m_shaderProgram = std::make_shared<ShaderProgram>("DrawFBO");
    m_shaderProgram->addShader(m_fragmentShader);
    m_shaderProgram->addShader(m_drawFBOShader);
    m_shaderProgram->link();
}

ScreenFillingQuad::~ScreenFillingQuad()
{

}

std::shared_ptr<ShaderProgram> ScreenFillingQuad::getShaderProgram()
{
    return m_shaderProgram;
}

void ScreenFillingQuad::draw()
{
    //m_shaderProgram->use();
    //m_shaderProgram->setVec2("resolution", glm::vec2(m_scene->getWidth(), m_scene->getHeight()));
    //m_shaderProgram->setSampler2D("fbo", fboBufferID, 0);
    m_quad->draw();
}