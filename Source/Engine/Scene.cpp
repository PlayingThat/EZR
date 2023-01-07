//
// Created by maxbr on 21.11.2022.
//

#include "Engine/Scene.h"
// Needed to resolve forward declaration
#include "Engine/Drawable.h"

Scene::Scene(std::shared_ptr<State> state)
{
    m_state = state;

}

void Scene::setup(std::shared_ptr<Scene> scene)
{
    m_drawables = std::vector<std::shared_ptr<Drawable>>();
    m_backgroundColor = std::make_unique<float[]>(3); 

    // Create FBO for GBuffer and SFQ
    m_gBufferFBO = std::make_shared<FBO>(scene, 3);
    m_sfq = std::make_shared<ScreenFillingQuad>(scene);

    // Setup shaders for GBuffer
    m_gBufferVertexShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.vert");
    m_gBufferFragmentShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.frag");
    m_gBufferShaderProgram = std::make_shared<ShaderProgram>("GBuffer");
    m_gBufferShaderProgram->addShader(m_gBufferVertexShader);
    m_gBufferShaderProgram->addShader(m_gBufferFragmentShader);
    m_gBufferShaderProgram->link();

    // Setup shader program for NP effects
    m_NPREffectShaderPrograms = std::vector<std::shared_ptr<ShaderProgram>>();
    setupNPREffects();

    // Setup objects
    m_triangle = std::make_shared<ColorfullTriangle>(scene);

    m_terrain = std::make_shared<Terrain>(scene);

    m_ghost = std::make_shared<Ghost>(scene);

    // Set camera position
    m_state->getCamera()->setPosition(glm::vec3(0.0f, 1.7f, 2.7f));

    //addObject(m_triangle);
    //addObject(m_terrain);
    addObject(m_ghost);
}

Scene::~Scene()
{

}

std::shared_ptr<State> Scene::getState() const
{
    return m_state;
}

void Scene::update(float deltaTime)
{
    // GUI
    ImGui::Begin("Performance");
    ImGui::Text("%s", std::string("Frame Time: " + std::to_string(deltaTime * 1000.0f) + "ms").c_str());
    ImGui::Text("%s", std::string("Frames per Second: " + std::to_string(1.0f / deltaTime)).c_str());
    ImGui::ColorPicker3("Background color", m_backgroundColor.get());
    ImGui::End();

    drawGeometry();
    drawSFQuad();
}

void Scene::setupNPREffects()
{
    // Setup Basic shader
    m_basicVertexShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.vert");
    m_basicFragmentShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.frag");
    m_basicShaderProgram = std::make_shared<ShaderProgram>("DrawFBO");
    m_basicShaderProgram->addShader(m_basicVertexShader);
    m_basicShaderProgram->addShader(m_basicFragmentShader);
    m_basicShaderProgram->link();
    m_NPREffectShaderPrograms.push_back(m_basicShaderProgram);
}

void Scene::drawGeometry()
{
    // Render to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO->getID());
    glClearColor(0.0, 0.0, 0.0, 1.0); // keep it black so it doesn't leak into g-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_gBufferShaderProgram->use();

    m_gBufferShaderProgram->setMat4("projectionMatrix", *getState()->getCamera()->getProjectionMatrix());
    m_gBufferShaderProgram->setMat4("viewMatrix", *getState()->getCamera()->getViewMatrix());

    renderDrawables();
}

void Scene::drawSFQuad()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_NPREffectShaderPrograms.at(0)->use();

    // Set shader uniforms
    m_NPREffectShaderPrograms.at(0)->setVec2("screenSize", glm::vec2(getState()->getCamera()->getWidth(),
                                                                     getState()->getCamera()->getHeight()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBufferFBO->getColorAttachment(0));  // position
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBufferFBO->getColorAttachment(0));  // normal
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gBufferFBO->getColorAttachment(0));  // albedo
    
    m_sfq->draw();  
}

void Scene::renderDrawables()
{
    for (std::shared_ptr<Drawable> d : m_drawables)
    {
        m_basicShaderProgram->setMat4("modelMatrix", d->getModelMatrix());
        m_basicShaderProgram->setMat3("normalMatrix", glm::mat3(
                                                glm::inverseTranspose(*getState()->getCamera()->getViewMatrix() * 
                                                d->getModelMatrix())));
        d->draw();
    }
}

void Scene::addObject(std::shared_ptr<Drawable> object)
{
    m_drawables.push_back(object);
}