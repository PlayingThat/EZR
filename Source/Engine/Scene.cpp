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
    m_scene = scene;
    m_drawables = std::vector<std::shared_ptr<Drawable>>();
    m_backgroundColor = std::make_unique<float[]>(3); 

    // Create FBO for GBuffer and SFQ
    m_gBufferFBO = std::make_shared<FBO>(m_scene, 3);
    m_sfq = std::make_shared<ScreenFillingQuad>(m_scene);

    // Setup shaders for GBuffer
    m_gBufferVertexShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.vert");
    m_gBufferFragmentShader = std::make_shared<Shader>("./Assets/Shader/GBuffer.frag");
    m_gBufferShaderProgram = std::make_shared<ShaderProgram>("GBuffer");
    m_gBufferShaderProgram->addShader(m_gBufferVertexShader);
    m_gBufferShaderProgram->addShader(m_gBufferFragmentShader);
    m_gBufferShaderProgram->link();

    // Setup shaders for compositing
    m_compositingVertexShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.vert");
    m_compositingFragmentShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.frag");
    m_compositingShaderProgram = std::make_shared<ShaderProgram>("Compositing");
    m_compositingShaderProgram->addShader(m_compositingVertexShader);
    m_compositingShaderProgram->addShader(m_compositingFragmentShader);
    m_compositingShaderProgram->link();

    // Setup shader program for NP effects
    m_NPREffects = std::vector<std::shared_ptr<NPREffect>>();
    // m_nprEffectNames = std::vector<std::string>();
    setupNPREffects();

    // Setup objects
    m_triangle = std::make_shared<ColorfullTriangle>(m_scene);

    m_terrain = std::make_shared<Terrain>(m_scene);

    m_ghost = std::make_shared<Ghost>(m_scene);

    // Set camera position
    getState()->getCamera()->setPosition(glm::vec3(0.0f, 1.7f, 2.7f));

    //addObject(m_triangle);
    addObject(m_terrain);
    addObject(m_ghost);
}

Scene::~Scene()
{

}

std::shared_ptr<State> Scene::getState() const
{
    return m_scene->m_state;
}

void Scene::update(float deltaTime)
{
    // GUI
    ImGui::Begin("Performance");
    ImGui::Text("%s", std::string("Frame Time: " + std::to_string(deltaTime * 1000.0f) + "ms").c_str());
    ImGui::Text("%s", std::string("Frames per Second: " + std::to_string(1.0f / deltaTime)).c_str());
    ImGui::ColorPicker3("Background color", m_backgroundColor.get());
    ImGui::End();

    drawNPRPanel();  
    drawGeometry();
    drawSFQuad();
}

void Scene::setupNPREffects()
{
    // Setup Basic shader
    m_basicVertexShader = std::make_shared<Shader>("./Assets/Shader/DrawFBO.vert");
    m_basicFragmentShader = std::make_shared<Shader>("./Assets/Shader/Basic.frag");
    m_basicShaderProgram = std::make_shared<ShaderProgram>("Color unlit");
    m_basicShaderProgram->addShader(m_basicVertexShader);
    m_basicShaderProgram->addShader(m_basicFragmentShader);
    m_basicShaderProgram->link();
    addNPREffect(m_basicShaderProgram, true);

    // Setup Gooch
    m_goochVertexShader = std::make_shared<Shader>("./Assets/Shader/Gooch.vert");
    m_goochFragmentShader = std::make_shared<Shader>("./Assets/Shader/Gooch.frag");
    m_goochShaderProgram = std::make_shared<ShaderProgram>("Gooch");
    m_goochShaderProgram->addShader(m_goochVertexShader);
    m_goochShaderProgram->addShader(m_goochFragmentShader);
    m_goochShaderProgram->link();
    addNPREffect(m_goochShaderProgram, false);
    addNPRProperty("Gooch", "textured", &m_goochPropertyTextured, true);

    // Setup Toon
    m_toonVertexShader = std::make_shared<Shader>("./Assets/Shader/Toon.vert");
    m_toonFragmentShader = std::make_shared<Shader>("./Assets/Shader/Toon.frag");
    m_toonShaderProgram = std::make_shared<ShaderProgram>("Toon");
    m_toonShaderProgram->addShader(m_toonVertexShader);
    m_toonShaderProgram->addShader(m_toonFragmentShader);
    m_toonShaderProgram->link();
    addNPREffect(m_toonShaderProgram, false);
    addNPRProperty("Toon", "textured", &m_toonPropertyTextured, false);
}

void Scene::addNPREffect(std::shared_ptr<ShaderProgram> nprEffectProgram, bool enabledByDefault)
{
    std::shared_ptr<NPREffect> nprEffect = std::make_shared<NPREffect>();
    std::shared_ptr<FBO> nprFBO = std::make_shared<FBO>(m_scene, 1);
    std::vector<NPRProperty> nprProperties = std::vector<NPRProperty>();
    nprEffect->shaderProgram = nprEffectProgram;
    nprEffect->enabled = enabledByDefault;
    nprEffect->name = nprEffectProgram->getName();
    nprEffect->fbo = nprFBO;
    nprEffect->properties = nprProperties;
    m_NPREffects.push_back(nprEffect);
}

void Scene::addNPRProperty(std::string effectName, std::string propertyName, std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value, bool showInGUI)
{
    // Iterate over all effects
    for (int i = 0; i < m_NPREffects.size(); i++)
    {
        // Find the effect with the given name
        if (m_NPREffects[i]->name == effectName)
        {
            // Add the property to the effect
            NPRProperty nprProperty = NPRProperty();
            nprProperty.name = propertyName;
            nprProperty.value = value;
            nprProperty.showInGUI = showInGUI;
            m_NPREffects[i]->properties.push_back(nprProperty);
            return;
        }
    }

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
    m_enabledNPREffectCount = 0;

    // Render NPR effects to their corresponding FBOs
    for (int i = 0; i < m_NPREffects.size(); i++)
    {
        if (m_NPREffects.at(i)->enabled)
        {
            m_enabledNPREffectCount++;

            glBindFramebuffer(GL_FRAMEBUFFER, m_NPREffects.at(i)->fbo->getID());
            glClearColor(0.0, 0.0, 0.0, 1.0);  // clear previous frame
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_NPREffects.at(i)->shaderProgram->use();

            // Set shader uniforms
            m_NPREffects.at(i)->shaderProgram->setVec2("screenSize", glm::vec2(getState()->getCamera()->getWidth(),
                                                                              getState()->getCamera()->getHeight()));
            
            m_NPREffects.at(i)->shaderProgram->setSampler2D("positions", 0, m_gBufferFBO->getColorAttachment(0));  // color diffuse
            m_NPREffects.at(i)->shaderProgram->setSampler2D("normals", 1, m_gBufferFBO->getColorAttachment(1));  // color diffuse
            m_NPREffects.at(i)->shaderProgram->setSampler2D("colorDiffuse", 2, m_gBufferFBO->getColorAttachment(2));  // color diffuse

            // Set NPR properties
            // Iterate over npr effect properties
            for(int effectPropertyIndex = 0; effectPropertyIndex < m_NPREffects.at(i)->properties.size(); effectPropertyIndex++)
            {
                setNPREffectProperty(m_NPREffects.at(i)->shaderProgram, 
                                     m_NPREffects.at(i)->properties.at(effectPropertyIndex).name,
                                     m_NPREffects.at(i)->properties.at(effectPropertyIndex).value);
            }
            
            // Draw quad
            m_sfq->draw();
        }
    }

    // Render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_compositingShaderProgram->use();
    m_compositingShaderProgram->setVec2("screenSize", glm::vec2(getState()->getCamera()->getWidth(),
                                                                     getState()->getCamera()->getHeight()));
    m_compositingShaderProgram->setInt("numberOfEnabledEffects", m_enabledNPREffectCount);

    // Set shader uniforms
    int shaderFBOOffset = 0;
    // Set all textures from npf effect FBOs
    for (int i = 0; i < m_NPREffects.size(); i++)
    {
        if (m_NPREffects.at(i)->enabled)
        {
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, m_NPREffects.at(i)->fbo->getColorAttachment(2));
            m_compositingShaderProgram->setSampler2D(std::string("fbo0"),
                                                     0 + shaderFBOOffset, m_NPREffects.at(i)->fbo->getColorAttachment(0));

            shaderFBOOffset++;
        }
    }
    
    m_sfq->draw();  
}

void Scene::renderDrawables()
{
    for (std::shared_ptr<Drawable> d : m_drawables)
    {
        m_gBufferShaderProgram->setMat4("modelMatrix", d->getModelMatrix());
        m_gBufferShaderProgram->setMat3("normalMatrix", glm::mat3(
                                                glm::inverseTranspose(*getState()->getCamera()->getViewMatrix() * 
                                                d->getModelMatrix())));
        d->draw();
    }
}

void Scene::addObject(std::shared_ptr<Drawable> object)
{
    m_drawables.push_back(object);
}

void Scene::drawNPRPanel()
{

    ImGui::Begin("NPR Effects");
    // int move_from = -1, move_to = -1;
    for (int n = 0; n < m_NPREffects.size(); n++)
    {
        //ImGui::Selectable(names[n]);
        ImGui::Checkbox(m_NPREffects.at(n)->name.c_str(), &m_NPREffects.at(n)->enabled);

        // Draw properties for each effect
        if (m_NPREffects.at(n)->properties.size() > 0) {
            ImGui::Indent();
            for(int effectPropertyIndex = 0; effectPropertyIndex < m_NPREffects.at(n)->properties.size(); effectPropertyIndex++)
            {
                // Only draw effect if it should be drawn to GUI
                if (m_NPREffects.at(n)->properties.at(effectPropertyIndex).showInGUI)
                    drawNPREffectProperty(m_NPREffects.at(n)->properties.at(effectPropertyIndex));
            }
            ImGui::Unindent();
        }



        // ImGuiDragDropFlags src_flags = 0;
        // src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
        // src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging
        // //src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
        // if (ImGui::BeginDragDropSource(src_flags))
        // {
        //     // if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
        //     //     ImGui::Text("Moving \"%s\"", m_NPREffects.at(n)->name.c_str());
        //     ImGui::SetDragDropPayload("DND_NPR_EFFECTS", &n, sizeof(int));
        //     ImGui::EndDragDropSource();
        // }

        // if (ImGui::BeginDragDropTarget())
        // {
        //     ImGuiDragDropFlags target_flags = 0;
        //     target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
        //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("-", target_flags))
        //     {
        //         int testString = 1;
        //         move_from = *(const int*)payload->Data;
        //         move_to = n;
        //     }
        //     ImGui::EndDragDropTarget();
        // }
    }

    // if (move_from != -1 && move_to != -1)
    // {
    //     // Reorder items
    //     int copy_dst = (move_from < move_to) ? move_from : move_to + 1;
    //     int copy_src = (move_from < move_to) ? move_from + 1 : move_to;
    //     int copy_count = (move_from < move_to) ? move_to - move_from : move_from - move_to;

    //     std::shared_ptr<NPREffect> temp = m_NPREffects[move_to];
    //     m_NPREffects[move_to] = m_NPREffects[move_from];
    //     m_NPREffects[move_from] = temp;
        
    //     ImGui::SetDragDropPayload("DND_NPR_EFFECTS", &move_to, sizeof(int)); // Update payload immediately so on the next frame if we move the mouse to an earlier item our index payload will be correct. This is odd and showcase how the DnD api isn't best presented in this example.
    // }
    ImGui::End();
}

void Scene::setNPREffectProperty(std::shared_ptr<ShaderProgram> shaderProgram, 
                                 std::string propertyName, 
                                 std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value)
{
    if (std::holds_alternative<glm::vec2*>(value))
    {
        shaderProgram->setVec2(propertyName, *std::get<glm::vec2*>(value));
    }
    else if (std::holds_alternative<glm::vec3*>(value))
    {
        shaderProgram->setVec3(propertyName, *std::get<glm::vec3*>(value));
    }
    else if (std::holds_alternative<glm::vec4*>(value))
    {
        shaderProgram->setVec4(propertyName, *std::get<glm::vec4*>(value));
    }
    else if (std::holds_alternative<float*>(value))
    {
        shaderProgram->setFloat(propertyName, *std::get<float*>(value));
    }
    else if (std::holds_alternative<int*>(value))
    {
        shaderProgram->setInt(propertyName, *std::get<int*>(value));
    }
    else if (std::holds_alternative<bool*>(value))
    {
        shaderProgram->setBool(propertyName, *std::get<bool*>(value));
    }
    else if (std::holds_alternative<GLuint*>(value))
    {
        shaderProgram->setSampler2D(propertyName, 0, *std::get<GLuint*>(value));
    }

}

void Scene::drawNPREffectProperty(NPRProperty property)
{
    std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value = property.value;
    std::string propertyName = property.name;

    // if (std::holds_alternative<glm::vec2*>(value))
    // {
    //     shaderProgram->setVec2(propertyName, *std::get<glm::vec2*>(value));
    //     ImGUI
    // }
    // else if (std::holds_alternative<glm::vec3*>(value))
    // {
    //     shaderProgram->setVec3(propertyName, *std::get<glm::vec3*>(value));
    // }
    // else if (std::holds_alternative<glm::vec4*>(value))
    // {
    //     shaderProgram->setVec4(propertyName, *std::get<glm::vec4*>(value));
    // }
    // else if (std::holds_alternative<float*>(value))
    // {
    //     shaderProgram->setFloat(propertyName, *std::get<float*>(value));
    // }
    // else if (std::holds_alternative<int*>(value))
    // {
    //     shaderProgram->setInt(propertyName, *std::get<int*>(value));
    // }
    if (std::holds_alternative<bool*>(value))
    {
        ImGui::Checkbox(propertyName.c_str(), std::get<bool*>(value));
    }
    // else if (std::holds_alternative<GLuint*>(value))
    // {
    //     shaderProgram->setSampler2D(propertyName, 0, *std::get<GLuint*>(value));
    // }
}
