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
    m_drawables = std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>>();
    m_transparentDrawables = std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>>();

    m_profilerWindow = std::make_shared<ProfilersWindow>();

    // Create FBO for GBuffer and SFQ
    m_gBufferFBO = std::make_shared<FBO>(m_scene, 8);
    m_gBufferTransparentFBO = std::make_shared<FBO>(m_scene, 8);
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
    m_sunPosition = std::shared_ptr<Clouds>(m_clouds, reinterpret_cast<Clouds *>(m_clouds.get()))->getSunPosition();
    m_sunColor = std::shared_ptr<Clouds>(m_clouds, reinterpret_cast<Clouds *>(m_clouds.get()))->getSunColor();

    // Setup scene objects
    m_ghost = std::make_shared<Ghost>(m_scene);
    m_gingerbreadHouse = std::make_shared<GingerbreadHouse>(m_scene);
    m_snowMan = std::make_shared<Snowman>(m_scene);
    m_stone1 = std::make_shared<Stone1>(m_scene);
    m_stone2 = std::make_shared<Stone2>(m_scene);
    m_stone3 = std::make_shared<Stone3>(m_scene);
    m_stone4 = std::make_shared<Stone4>(m_scene);
    m_treePlain = std::make_shared<TreePlain>(m_scene);
    m_treeLeaves = std::make_shared<TreeLeaves>(m_scene);
    
    // Set camera position
    getState()->getCamera()->setPosition(glm::vec3(0.0f, 1.7f, 2.7f));

    //addObject(m_triangle);
    addObject(m_terrain);
    addObject(m_clouds);
    addObject(m_ghost, true, Transformation{glm::vec3(1, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_ghost, true, Transformation{glm::vec3(4, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_ghost, false, Transformation{glm::vec3(-4, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});

    addObject(m_gingerbreadHouse, false, Transformation{glm::vec3(-3, -8, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_snowMan, false, Transformation{glm::vec3(1, -4, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_stone1, false, Transformation{glm::vec3(-3, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_stone2, false, Transformation{glm::vec3(-2, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_stone3, false, Transformation{glm::vec3(-1, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_stone4, false, Transformation{glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_treePlain, false, Transformation{glm::vec3(-8, -4, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});
    addObject(m_treeLeaves, false, Transformation{glm::vec3(3, -6, 0), glm::vec3(1, 1, 1), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f});

    loadMapTexture();
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

    applyNPREffects(m_gBufferFBO);
    // if (m_transparency)
    //     applyNPREffects(m_gBufferTransparentFBO);
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
    addNPRProperty("Gooch", "CoolColor##Gooch", &m_goochPropertyCoolColor, true);
    addNPRProperty("Gooch", "DiffuseCool##Gooch", &m_goochPropertyDiffuseCool, true);
    addNPRProperty("Gooch", "WarmColor##Gooch", &m_goochPropertyWarmColor, true);
    addNPRProperty("Gooch", "DiffuseWarm##Gooch", &m_goochPropertyDiffuseWarm, true);   
    addNPRProperty("Gooch", "Textured##Gooch", &m_goochPropertyTextured, true);
    addNPRProperty("Gooch", "UseSun##Gooch", &m_goochPropertyUseSun, true);

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
    addNPRProperty("Toon", "colorLevels##JToon", &m_JtoonPropertyColorLevels, true, 1, 20);
    addNPRProperty("Toon", "levelBrightness##JToon", &m_JtoonPropertyLevelBrightness, true);
    addNPRProperty("Toon", "Textured##JToon", &m_JtoonPropertyTextured, true);
    addNPRProperty("Toon", "UseSun##JToon", &m_JtoonPropertyUseSun, true);
    addNPRProperty("Toon", "SunlightInfluence##JToon", &m_JtoonPropertySunlightInfluence, true);

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
    addNPRProperty("Patterns", "mode##Patterns", &m_pattPropertyMode, true, 0, 5);
    addNPRProperty("Patterns", "frequency##Patterns", &m_pattPropertyFrequency, true, 0.5, 2.0);
    //addNPRProperty("Patterns", "noiseActive", &m_pattPropertyNoiseActive, true);
    addNPRProperty("Patterns", "noiseFactor##Patterns", &m_pattPropertyNoiseFactor, true, 0.0, 0.5);
    addNPRProperty("Patterns", "Colored##Patterns", &m_pattPropertyColored, true);
    addNPRProperty("Patterns", "Textured##Patterns", &m_pattPropertyTextured, true);
    addNPRProperty("Patterns", "UseSun##Patterns", &m_pattPropertyUseSun, true);
    addNPRProperty("Patterns", "SunlightInfluence##Patterns", &m_pattPropertySunlightInfluence, true);

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
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    m_gBufferShaderProgram->use();

    m_gBufferShaderProgram->setFloat("HeightScale", m_parallaxMappingHeightScale);
    m_gBufferShaderProgram->setBool("UseParallaxMapping", m_useParallaxMapping);
    m_gBufferShaderProgram->setVec3("cameraPosition", getState()->getCamera()->getPosition());
    m_gBufferShaderProgram->setMat4("projectionMatrix", *getState()->getCamera()->getProjectionMatrix());
    m_gBufferShaderProgram->setMat4("viewMatrix", *getState()->getCamera()->getViewMatrix());
    m_gBufferShaderProgram->setBool("isTransparent", false);

    renderDrawables(m_drawables, m_gBufferFBO);
    m_profilerWindow->EndCPUProfilerTask("drawGeometry");

    /////////////////////////////////////////////
    // Draw transparent objects

    if (m_transparency)
        drawTransparentGeometry();
    else
        renderDrawables(m_transparentDrawables, m_gBufferFBO);
}

void Scene::drawTransparentGeometry()
{
    m_profilerWindow->StartCPUProfilerTask("drawTransparentGeometry");

    // Copy gbuffer to transparent gbuffer
    // m_gBufferFBO->copyToFBO(m_gBufferTransparentFBO);
    // Render to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferTransparentFBO->getID());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_ALWAYS);

    m_gBufferShaderProgram->use();

    m_gBufferShaderProgram->setFloat("HeightScale", m_parallaxMappingHeightScale);
    m_gBufferShaderProgram->setBool("UseParallaxMapping", m_useParallaxMapping);
    m_gBufferShaderProgram->setVec3("cameraPosition", getState()->getCamera()->getPosition());
    m_gBufferShaderProgram->setMat4("projectionMatrix", *getState()->getCamera()->getProjectionMatrix());
    m_gBufferShaderProgram->setMat4("viewMatrix", *getState()->getCamera()->getViewMatrix());
    m_gBufferShaderProgram->setBool("isTransparent", true);

    renderDrawables(m_transparentDrawables, m_gBufferTransparentFBO);

    // Reset GL state
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_profilerWindow->EndCPUProfilerTask("drawTransparentGeometry");
}

void Scene::applyNPREffects(std::shared_ptr<FBO> fbo)
{
    m_profilerWindow->StartCPUProfilerTask("applyNPREffects");
 
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
            
            m_NPREffects.at(i)->shaderProgram->setSampler2D("positions", 0, fbo->getColorAttachment(0)); 
            m_NPREffects.at(i)->shaderProgram->setSampler2D("normals", 1, fbo->getColorAttachment(1));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("uvs", 2, fbo->getColorAttachment(2));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("tangents", 3, fbo->getColorAttachment(3));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureDiffuse", 4, fbo->getColorAttachment(4)); 
            m_NPREffects.at(i)->shaderProgram->setSampler2D("colorDiffuse", 5, fbo->getColorAttachment(5));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("depth", 6, fbo->getDepthAttachment());
             
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureMetalSmoothnessAOHeight", 7, fbo->getColorAttachment(6));  
            m_NPREffects.at(i)->shaderProgram->setSampler2D("textureNormal", 8, fbo->getColorAttachment(7));  

            m_NPREffects.at(i)->shaderProgram->setVec3("cameraPosition", glm::vec3(getState()->getCamera()->getPosition()));  // camera
            m_NPREffects.at(i)->shaderProgram->setVec3("lightPosition", *m_sunPosition);  // sun position
            m_NPREffects.at(i)->shaderProgram->setVec3("lightColor", glm::vec3(*m_sunColor));  // sun color

            // Stippling Shader Textures
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp1", 115, m_stipp1);  // index offset for ao, metallic, roughness, and possibly bitangents if needed
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp2", 116, m_stipp2);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp3", 117, m_stipp3);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp4", 118, m_stipp4);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp5", 119, m_stipp5);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("stipp6", 120, m_stipp6);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("paper", 121, m_paper);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("noise", 120, m_noise);
            m_NPREffects.at(i)->shaderProgram->setSampler2D("canvas", 121, m_canvas);

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
    
    m_profilerWindow->EndCPUProfilerTask("applyNPREffects");
}

void Scene::drawSFQuad()
{
    
    m_profilerWindow->StartCPUProfilerTask("finalCompositing");

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
    
    m_compositingShaderProgram->setBool("transparency", m_transparency);
    if (m_transparency) {
        m_compositingShaderProgram->setSampler2D("transparencySampler", 15, m_gBufferTransparentFBO->getColorAttachment(2)); 
        m_compositingShaderProgram->setSampler2D("transparencyTextureSampler", 16, m_gBufferTransparentFBO->getColorAttachment(4)); 
        m_compositingShaderProgram->setSampler2D("transparencyDiffuseSampler", 17, m_gBufferTransparentFBO->getColorAttachment(5));
        m_compositingShaderProgram->setSampler2D("transparencyDepth", 18, m_gBufferTransparentFBO->getDepthAttachment());
    }

    // Set shader uniforms
    int shaderFBOOffset = 0;
    // Set all textures from npf effect FBOs
    for (int i = 0; i < m_NPREffects.size(); i++)
    {
        if (m_NPREffects.at(i)->enabled)
        {
            m_compositingShaderProgram->setSampler2D(std::string("fbo0"),
                                                     0 + shaderFBOOffset, m_NPREffects.at(i)->fbo->getColorAttachment(0));

            shaderFBOOffset++;
        }
    }
    
    m_sfq->draw();  
    
    m_profilerWindow->EndCPUProfilerTask("finalCompositing");
}

void Scene::renderDrawables(std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>> drawables, std::shared_ptr<FBO> fbo)
{
    for (auto& el : drawables)
    {
        std::shared_ptr<Drawable> d = el.first;
        Transformation t = el.second;
        d->setBasePosition(t.position);
        d->setBaseRotation(t.rotationAxis, t.rotationAngle);
        d->setBaseScale(t.scale);

        // Terrain and clouds may change the FBO and shader
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->getID());
        m_gBufferShaderProgram->use();

        m_gBufferShaderProgram->setMat4("modelMatrix", d->getModelMatrix());
        m_gBufferShaderProgram->setMat3("normalMatrix", glm::mat3(
                                                glm::inverseTranspose(*getState()->getCamera()->getViewMatrix() * 
                                                d->getModelMatrix())));
        d->draw();
    }
}

void Scene::addObject(std::shared_ptr<Drawable> object,
                   bool transparent,
                   Transformation transform)
{
    if (transparent)
        m_transparentDrawables.push_back({object, transform});
    else
        m_drawables.push_back({object, transform});
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
    ImGui::Checkbox("Transparency", &m_transparency);
    ImGui::Separator();
    
    bool noEffectEnabled = true;
    for (int n = 0; n < m_NPREffects.size(); n++)
    {
        noEffectEnabled = noEffectEnabled && !m_NPREffects.at(n)->enabled;
        //ImGui::Selectable(names[n]);
        ImGui::Checkbox(m_NPREffects.at(n)->name.c_str(), &m_NPREffects.at(n)->enabled);

        // Draw properties for each effect
        //if (m_NPREffects.at(n)->properties.size() > 0) {
        if (m_NPREffects.at(n)->enabled == true && m_NPREffects.at(n)->properties.size() > 0) {
            ImGui::Indent();
            for(int effectPropertyIndex = 0; effectPropertyIndex < m_NPREffects.at(n)->properties.size(); effectPropertyIndex++)
            {
                // Only draw effect if it should be drawn to GUI
                if (m_NPREffects.at(n)->properties.at(effectPropertyIndex).showInGUI)
                    drawNPREffectProperty(m_NPREffects.at(n)->properties.at(effectPropertyIndex));
            }
            ImGui::Unindent();
        }
        ImGui::Separator();
    }

    // Dont fill object with last bound texture
    if (noEffectEnabled)
    {
        m_NPREffects.at(0)->enabled = true;
    }

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
        ImGui::ColorEdit3(propertyName.c_str(), glm::value_ptr(*std::get<glm::vec3*>(value)));
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

void Scene::loadMapTexture()
{
    // Map between the texture rgb pixels and the object type
    std::map<std::tuple<int, int, int>, std::shared_ptr<Drawable>> pixelToObjectMap = {
        { std::make_tuple(41, 253, 47), m_treeLeaves }
    };

    // Load map texture
    int width, height;

    Pixel* map = loadTextureFromFileDirect("./Assets/Terrain/mapSmall.png", width, height);
    float scaleOffset = 1.0f;
    glm::vec3 offset = glm::vec3(0, -800, 0);

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            // Skip white pixels
            if (map[i * height + j].red == 255 && map[i * height + j].green == 255 && map[i * height + j].blue == 255)
                continue;

            std::tuple tmp = std::make_tuple(map[i * height + j].red, map[i * height + j].green, map[i * height + j].blue);
            if (pixelToObjectMap.find(tmp) != pixelToObjectMap.end()) {
                Transformation t = Transformation { offset + glm::vec3(j, i, 0) * scaleOffset, glm::vec3(1.0f), glm::vec3(1, 0, 0), 90.0f };
                addObject(pixelToObjectMap[tmp], false, t);
            }
            else {
                LOG_INFO("Unknown map texture rgb value " + std::to_string(map[i * height + j].red) + " " + std::to_string(map[i * height + j].green) + " " + std::to_string(map[i * height + j].blue));
            }
        }
    }
}
