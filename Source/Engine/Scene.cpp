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

    m_profilerWindow = std::make_shared<ProfilersWindow>();

    // Create FBO for GBuffer and SFQ
    m_gBufferFBO = std::make_shared<FBO>(m_scene, 8);
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

    /////////////////////////////////////////////
    // Setup objects
    m_triangle = std::make_shared<ColorfullTriangle>(m_scene);

    m_terrain = std::make_shared<Terrain>(m_scene);
    m_terrainTextures = reinterpret_cast<Terrain *>(m_terrain.get())->getDrawTextures();

    m_clouds = std::make_shared<Clouds>(m_scene);
    m_cloudColorTexture = std::shared_ptr<Clouds>(m_clouds, reinterpret_cast<Clouds *>(m_clouds.get()))->getCloudTexture();

    // Setup scene objects
    m_ghost = std::make_shared<Ghost>(m_scene);
    
    // Set camera position
    getState()->getCamera()->setPosition(glm::vec3(0.0f, 1.7f, 2.7f));

    //addObject(m_triangle);
    LOG_INFO("Try to add terrain");
    addObject(m_terrain);
    LOG_INFO("Try to add clouds");
    addObject(m_clouds);
    LOG_INFO("Try to add ghost");
    addObject(m_ghost);
    LOG_INFO("Finished with adding");
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
    m_profilerWindow->Render();

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

    // Setup Gooch (by Jessica)
    m_goochVertexShader = std::make_shared<Shader>("./Assets/Shader/Gooch.vert");
    m_goochFragmentShader = std::make_shared<Shader>("./Assets/Shader/Gooch.frag");
    m_goochShaderProgram = std::make_shared<ShaderProgram>("Gooch");
    m_goochShaderProgram->addShader(m_goochVertexShader);
    m_goochShaderProgram->addShader(m_goochFragmentShader);
    m_goochShaderProgram->link();
    addNPREffect(m_goochShaderProgram, false);
    addNPRProperty("Gooch", "Textured##Gooch", &m_goochPropertyTextured, true);
    addNPRProperty("Gooch", "CoolColor", &m_goochPropertyCoolColor, true);
    addNPRProperty("Gooch", "WarmColor", &m_goochPropertyWarmColor, true);
    

    // Setup Toon with outline (by Alyssa)
    m_toonVertexShader = std::make_shared<Shader>("./Assets/Shader/Toon.vert");
    m_toonFragmentShader = std::make_shared<Shader>("./Assets/Shader/Toon.frag");
    m_toonShaderProgram = std::make_shared<ShaderProgram>("Toon with Outline");
    m_toonShaderProgram->addShader(m_toonVertexShader);
    m_toonShaderProgram->addShader(m_toonFragmentShader);
    m_toonShaderProgram->link();
    addNPREffect(m_toonShaderProgram, false);
    addNPRProperty("Toon with Outline", "Textured##Toon", &m_toonPropertyTextured, true);

    // Setup Alternative Toon with adjustable parameters (by Jessica)
    m_JtoonVertexShader = std::make_shared<Shader>("./Assets/Shader/JToon.vert");
    m_JtoonFragmentShader = std::make_shared<Shader>("./Assets/Shader/JToon.frag");
    m_JtoonShaderProgram = std::make_shared<ShaderProgram>("Toon");
    m_JtoonShaderProgram->addShader(m_JtoonVertexShader);
    m_JtoonShaderProgram->addShader(m_JtoonFragmentShader);
    m_JtoonShaderProgram->link();
    addNPREffect(m_JtoonShaderProgram, false);
    addNPRProperty("Toon", "Textured##JToon", &m_JtoonPropertyTextured, true);
    addNPRProperty("Toon", "colorLevels", &m_JtoonPropertyColorLevels, true, 1, 20);
    addNPRProperty("Toon", "levelBrightness", &m_JtoonPropertyLevelBrightness, true);

    // Setup Rim Lighting (by Alyssa)
    m_rimLVertexShader = std::make_shared<Shader>("./Assets/Shader/RimLighting.vert");
    m_rimLFragmentShader = std::make_shared<Shader>("./Assets/Shader/RimLighting.frag");
    m_rimLShaderProgram = std::make_shared<ShaderProgram>("RimLighting");
    m_rimLShaderProgram->addShader(m_rimLVertexShader);
    m_rimLShaderProgram->addShader(m_rimLFragmentShader);
    m_rimLShaderProgram->link();
    addNPREffect(m_rimLShaderProgram, false);
    addNPRProperty("RimLighting", "Textured##RimLighing", &m_rimLPropertyTextured, true);
    addNPRProperty("RimLighting", "strength", &m_rimLightStrength, true, 2.0f, 10.0f);

    // Set up alternative glow effect (by Jessica)
    m_glowVertexShader = std::make_shared<Shader>("./Assets/Shader/JGlow.vert");
    m_glowFragmentShader = std::make_shared<Shader>("./Assets/Shader/JGlow.frag");
    m_glowShaderProgram = std::make_shared<ShaderProgram>("Glow");
    m_glowShaderProgram->addShader(m_glowVertexShader);
    m_glowShaderProgram->addShader(m_glowFragmentShader);
    m_glowShaderProgram->link();
    addNPREffect(m_glowShaderProgram, false);
    addNPRProperty("JGlow", "Textured##JGlow", &m_glowPropertyTextured, true);

    // Setup Outline (by Alyssa)
    m_outlVertexShader = std::make_shared<Shader>("./Assets/Shader/Outline.vert");
    m_outlFragmentShader = std::make_shared<Shader>("./Assets/Shader/Outline.frag");
    m_outlShaderProgram = std::make_shared<ShaderProgram>("Outline");
    m_outlShaderProgram->addShader(m_outlVertexShader);
    m_outlShaderProgram->addShader(m_outlFragmentShader);
    m_outlShaderProgram->link();
    addNPREffect(m_outlShaderProgram, false);

    // Setup Watercolor (by Alyssa)
    m_waterColVertexShader = std::make_shared<Shader>("./Assets/Shader/Watercolor.vert");
    m_waterColFragmentShader = std::make_shared<Shader>("./Assets/Shader/Watercolor.frag");
    m_waterColShaderProgram = std::make_shared<ShaderProgram>("Watercolor");
    m_waterColShaderProgram->addShader(m_waterColVertexShader);
    m_waterColShaderProgram->addShader(m_waterColFragmentShader);
    m_waterColShaderProgram->link();
    addNPREffect(m_waterColShaderProgram, false);
    
    // Setup Pattern Shader (by Jessica)
    m_pattVertexShader = std::make_shared<Shader>("./Assets/Shader/Patterns.vert");
    m_pattFragmentShader = std::make_shared<Shader>("./Assets/Shader/Patterns.frag");
    m_pattShaderProgram = std::make_shared<ShaderProgram>("Patterns");
    m_pattShaderProgram->addShader(m_pattVertexShader);
    m_pattShaderProgram->addShader(m_pattFragmentShader);
    m_pattShaderProgram->link();
    addNPREffect(m_pattShaderProgram, false);
    addNPRProperty("Patterns", "Colored", &m_pattPropertyColored, true);
    addNPRProperty("Patterns", "Textured##Hatching", &m_pattPropertyTextured, true);
    addNPRProperty("Patterns", "mode", &m_pattPropertyMode, true, 0, 5);
    addNPRProperty("Patterns", "frequency", &m_pattPropertyFrequency, true, 0.5, 2.0);
    //addNPRProperty("Patterns", "noiseActive", &m_pattPropertyNoiseActive, true);
    addNPRProperty("Patterns", "noiseFactor", &m_pattPropertyNoiseFactor, true, 0.0, 0.5);

    // Setup PBR shader
    m_pbrVertexShader = std::make_shared<Shader>("./Assets/Shader/PBR.vert");
    m_pbrFragmentShader = std::make_shared<Shader>("./Assets/Shader/PBR.frag");
    m_pbrShaderProgram = std::make_shared<ShaderProgram>("PBR");
    m_pbrShaderProgram->addShader(m_pbrVertexShader);
    m_pbrShaderProgram->addShader(m_pbrFragmentShader);
    m_pbrShaderProgram->link();
    addNPREffect(m_pbrShaderProgram, false);
    

    // Stippling Textures
    createStipplingTexture();
    HANDLE_GL_ERRORS("setting up NPR effects");

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

void Scene::addNPRProperty(std::string effectName, std::string propertyName, 
                           std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value,
                           bool showInGUI,
                           float min,
                           float max)
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
            nprProperty.min = min;
            nprProperty.max = max;
            m_NPREffects[i]->properties.push_back(nprProperty);
            return;
        }
    }

}

void Scene::drawGeometry()
{
    m_profilerWindow->StartCPUProfilerTask("drawGeometry");
    // Render to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO->getID());
    glClearColor(0.0, 0.0, 0.0, 1.0); // keep it black so it doesn't leak into g-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_gBufferShaderProgram->use();

    m_gBufferShaderProgram->setFloat("HeightScale", m_parallaxMappingHeightScale);
    m_gBufferShaderProgram->setBool("UseParallaxMapping", m_useParallaxMapping);
    m_gBufferShaderProgram->setVec3("cameraPosition", getState()->getCamera()->getPosition());
    m_gBufferShaderProgram->setMat4("projectionMatrix", *getState()->getCamera()->getProjectionMatrix());
    m_gBufferShaderProgram->setMat4("viewMatrix", *getState()->getCamera()->getViewMatrix());

    renderDrawables();
    m_profilerWindow->EndCPUProfilerTask("drawGeometry");
}

void Scene::drawSFQuad()
{
    m_profilerWindow->StartCPUProfilerTask("drawNPREffect");
 
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
            
            m_NPREffects.at(i)->shaderProgram->setSampler2D("positions", 0, m_gBufferFBO->getColorAttachment(0)); 
            m_NPREffects.at(i)->shaderProgram->setSampler2D("normals", 1, m_gBufferFBO->getColorAttachment(1));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("uvs", 2, m_gBufferFBO->getColorAttachment(2));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("tangents", 3, m_gBufferFBO->getColorAttachment(3));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureDiffuse", 4, m_gBufferFBO->getColorAttachment(4)); 
            m_NPREffects.at(i)->shaderProgram->setSampler2D("colorDiffuse", 5, m_gBufferFBO->getColorAttachment(5));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("depth", 6, m_gBufferFBO->getDepthAttachment());
             
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureMetalSmoothnessAOHeight", 7, m_gBufferFBO->getColorAttachment(6));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureNormal", 8, m_gBufferFBO->getColorAttachment(7));  

            m_NPREffects.at(i)->shaderProgram->setVec3("cameraPosition", glm::vec3(getState()->getCamera()->getPosition()));  // camera
            m_NPREffects.at(i)->shaderProgram->setVec3("lightPosition", glm::vec3(100, 1000, 500));  // light

            // Stippling Shader Textures
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp1", 15, m_stipp1);  // index offset for ao, metallic, roughness, and possibly bitangents if needed
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp2", 16, m_stipp2);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp3", 17, m_stipp3);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp4", 18, m_stipp4);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp5", 19, m_stipp5);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp6", 20, m_stipp6);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("paper", 21, m_paper);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("noise", 20, m_noise);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("canvas", 21, m_canvas);

            // Set NPR properties
            // Iterate over npr effect properties
            for(int effectPropertyIndex = 0; effectPropertyIndex < m_NPREffects.at(i)->properties.size(); effectPropertyIndex++)
            {
                // Remove ID hashtags from property name and cast it to lowercase string for use in shader
                std::string cleanName = splitString(m_NPREffects.at(i)->properties.at(effectPropertyIndex).name, '#');
                setNPREffectProperty(m_NPREffects.at(i)->shaderProgram, 
                                     cleanName,
                                     m_NPREffects.at(i)->properties.at(effectPropertyIndex).value);
            }
            
            // Draw quad
            m_sfq->draw();
        }
    }
    
    m_profilerWindow->EndCPUProfilerTask("drawNPREffect");
    // m_profilerWindow->StartCPUProfilerTask("finalCompositing");

    // Render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_compositingShaderProgram->use();
    m_compositingShaderProgram->setVec2("screenSize", glm::vec2(getState()->getCamera()->getWidth(),
                                                                     getState()->getCamera()->getHeight()));
    m_compositingShaderProgram->setInt("numberOfEnabledEffects", m_enabledNPREffectCount);
    m_compositingShaderProgram->setSampler2D("fboClouds", 8, m_cloudColorTexture);  // color diffuse
    m_compositingShaderProgram->setSampler2D("depth", 9, m_gBufferFBO->getDepthAttachment());  // depth
    m_compositingShaderProgram->setSampler2D("terrain", 10, m_terrainTextures[0]);  // terrain color
    m_compositingShaderProgram->setSampler2D("terrainDepth", 11, m_terrainTextures[1]);  // terrain depth
    m_compositingShaderProgram->setSampler2D("terrainTopView", 12, m_terrainTextures[2]);  // terrain top view

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
    
    // m_profilerWindow->EndCPUProfilerTask("finalCompositing");
}

void Scene::renderDrawables()
{
    for (std::shared_ptr<Drawable> d : m_drawables)
    {
        // Terrain and clouds may change the FBO and shader
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO->getID());
        m_gBufferShaderProgram->use();

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

void Scene::createStipplingTexture()
{
    m_stipp1 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp1.jpg");
    m_stipp2 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp2.jpg");
    m_stipp3 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp3.jpg");
    m_stipp4 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp4.jpg");
    m_stipp5 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp5.jpg");
    m_stipp6 = createTextureFromFile("Assets/Relevant-Textures/Stippling/stipp6.jpg");
    m_paper = createTextureFromFile("Assets/Relevant-Textures/Stippling/paper.jpg");
    m_noise = createTextureFromFile("Assets/Relevant-Textures/Stippling/static.png");
    m_canvas = createTextureFromFile("Assets/Relevant-Textures/Stippling/canvas.png");
}

void Scene::drawNPRPanel()
{

    ImGui::Begin("NPR Effects");

    // Add General Category
    ImGui::Text("General");
    ImGui::Separator();
    ImGui::Checkbox("Use Parallax Mapping", &m_useParallaxMapping);
    ImGui::SliderFloat("Parallax Height Scale", &m_parallaxMappingHeightScale, 0.0f, 0.02f);
    ImGui::Separator();
    
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

    if (std::holds_alternative<glm::vec2*>(value))
    {
        ImGui::SliderFloat4(propertyName.c_str(), glm::value_ptr(*std::get<glm::vec2*>(value)), property.min, property.max);
    }
    else if (std::holds_alternative<glm::vec3*>(value))
    {
        ImGui::ColorPicker3(propertyName.c_str(), glm::value_ptr(*std::get<glm::vec3*>(value)));
    }
    else if (std::holds_alternative<glm::vec4*>(value))
    {
        ImGui::SliderFloat4(propertyName.c_str(), glm::value_ptr(*std::get<glm::vec4*>(value)), property.min, property.max);
    }

    else if (std::holds_alternative<float*>(value))
    {
        ImGui::SliderFloat(propertyName.c_str(), std::get<float*>(value), property.min, property.max);
    }

    else if (std::holds_alternative<int*>(value))
    {
        ImGui::SliderInt(propertyName.c_str(), std::get<int*>(value), (int)property.min, (int)property.max);
    }
    else if (std::holds_alternative<bool*>(value))
    {
        ImGui::Checkbox(propertyName.c_str(), std::get<bool*>(value));
    }
    // else if (std::holds_alternative<GLuint*>(value))
    // {
    //     shaderProgram->setSampler2D(propertyName, 0, *std::get<GLuint*>(value));
    // }
}


// Splits a string at a given delimiter and returns the first part
std::string Scene::splitString(std::string s, char del)
{
    std::stringstream ss(s);
    std::string result;
    std::getline(ss, result, del);
    return result;
}

std::shared_ptr<ProfilersWindow> Scene::getProfilerWindow()
{
    return m_profilerWindow;
}
