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
    m_gBufferFBO = std::make_unique<FBO>(scene, 3);
    m_sfq = std::make_shared<ScreenFillingQuad>(scene);

    // Setup shaders
    m_gBufferVertexShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.vert");
    m_gBufferFragmentShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.frag");
    m_gBufferShaderProgram = std::make_unique<ShaderProgram>("GBuffer");
    m_gBufferShaderProgram->addShader(m_gBufferVertexShader);
    m_gBufferShaderProgram->addShader(m_gBufferFragmentShader);

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

    // Reset scene and draw background color
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(m_backgroundColor.get()[0], m_backgroundColor.get()[1], m_backgroundColor.get()[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderDrawables();
}

void Scene::drawGeometry()
{
    // Render to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO->getID());
    glClearColor(0.0, 0.0, 0.0, 1.0); // keep it black so it doesn't leak into g-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_gBufferShaderProgram->use();

    renderDrawables();
}

void Scene::renderDrawables()
{
    for (std::shared_ptr<Drawable> d : m_drawables)
    {
        d->draw();
    }
}

void Scene::addObject(std::shared_ptr<Drawable> object)
{
    m_drawables.push_back(object);
}