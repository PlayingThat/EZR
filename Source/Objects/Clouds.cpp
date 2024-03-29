//
// Created by maxbr on 10.01.2023.
//

#include "Objects/Clouds.h"

Clouds::Clouds(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_PerlinWorleyTexture = 0;
    m_worleyTexture = 0;
    m_scene = scene;

    m_width = m_scene->getState()->getWidth();
    m_height = m_scene->getState()->getHeight();

    // setup default scene properties
    m_sunPosition = glm::vec3(1000.0f, 1000.0f, 1000.0f);

    m_backgroundColor = glm::vec3(0.47f, 0.55f, 0.86f);

    /* sky colors
    m_dayColor = glm::vec3(0.53f, 0.92f, 1.0f);         // light blue
    m_sunsetColor = glm::vec3(0.81f, 0.33f, 0.2f);      // blood-orange
    m_nightColor = glm::vec3(0.04f, 0.03f, 0.17f);      // dark blue
    */

    m_cloudScale = 100000.0f;
    m_coverageFactor = 1.0f;
    m_curliness = 0.5f;

    m_earthRadius = 600000.0f;

    m_startTime = glfwGetTime();

    initShaders();
    initSunColorSSBO();
    initTextures();
}

void Clouds::draw()
{
    m_scene->getProfilerWindow()->StartGPUProfilerTask("clouds");
    update();

    m_volumetricCloudsComputeShaderProgram->use();

    bindTexture2D(m_fragmentColorTextureClouds, 0);

    //  set uniform variables for cloud compute shader
    m_volumetricCloudsComputeShaderProgram->setInt("fragColorImage", 0);
    m_volumetricCloudsComputeShaderProgram->setSampler3D("perlinWorleySampler", m_PerlinWorleyTexture, 1);
    m_volumetricCloudsComputeShaderProgram->setSampler3D("worleySampler", m_worleyTexture, 2);
    // m_volumetricCloudsComputeShaderProgram->setSampler3D("weatherSampler", m_weather->getWeatherTexture(), 3);
    // if(m_useComplexWeather)
    //     m_volumetricCloudsComputeShaderProgram->setSampler3D("weatherSampler2", m_weather->getWeatherTexture2(), 4);
    m_volumetricCloudsComputeShaderProgram->setMat4("inverseProjectionMatrix",
                                                    glm::inverse(*m_scene->getState()->getCamera()->getProjectionMatrix()));
    m_volumetricCloudsComputeShaderProgram->setMat4("inverseViewMatrix",
                                                    glm::inverse(*m_scene->getState()->getCamera()->getViewMatrix()));
    m_volumetricCloudsComputeShaderProgram->setVec3("cameraPosition", *m_scene->getState()->getCamera()->getCameraPosition());

    m_volumetricCloudsComputeShaderProgram->setVec2("resolution", glm::vec2(m_scene->getState()->getWidth(), m_scene->getState()->getHeight()));
    m_volumetricCloudsComputeShaderProgram->setVec3("sunPosition", m_sunPosition);
    m_volumetricCloudsComputeShaderProgram->setVec3("backgroundColor", m_backgroundColor);
    m_volumetricCloudsComputeShaderProgram->setFloat("cloudScale", m_cloudScale);
    m_volumetricCloudsComputeShaderProgram->setFloat("earthRadius", m_earthRadius);
    m_volumetricCloudsComputeShaderProgram->setFloat("innerRadius", m_earthRadius + 10000.0f);
    m_volumetricCloudsComputeShaderProgram->setFloat("outerRadius", m_earthRadius + 20000.0f);

    m_volumetricCloudsComputeShaderProgram->setFloat("coverageFactor", m_coverageFactor);
    m_volumetricCloudsComputeShaderProgram->setFloat("curliness", m_curliness);
    // m_volumetricCloudsComputeShaderProgram->setInt("useComplexWeather", m_useComplexWeather);

    m_volumetricCloudsComputeShaderProgram->setFloat("timeOfDay", m_timeOfDay);
    m_volumetricCloudsComputeShaderProgram->setBool("timeLapsActivated", m_timeLapsActivated);
    m_volumetricCloudsComputeShaderProgram->setFloat("timespeed", m_timespeed);
    m_volumetricCloudsComputeShaderProgram->setFloat("time", glfwGetTime() - m_startTime);  // Delta time
    if (m_timeLapsActivated)
        m_volumetricCloudsComputeShaderProgram->setFloat("windSpeed", m_windSpeed * 100.0f);
    else
        m_volumetricCloudsComputeShaderProgram->setFloat("windSpeed", m_windSpeed );

    // if the user has activated timeLaps, the day-night cycle runs automatically
    if(m_timeLapsActivated == true){
        if (m_timeOfDay >= 24.0f){
            m_timeOfDay = 0.0f;
        }
        else {
            m_timeOfDay += m_timespeed * 100.0f * m_scene->getState()->getDeltaTime();
        }
    }

    // Bind sun color SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                7,
                m_sunColorSSBO);


    glDispatchCompute(m_width / 16, m_height / 16, 1);
    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // apply post processing before drawing
    m_cloudPostProcessingShaderProgram->use();
    bindTexture2D(m_fragmentColorTextureCloudsBlured, 0);
    m_cloudPostProcessingShaderProgram->setInt("fboOut", 0);
    m_cloudPostProcessingShaderProgram->setSampler2D("cloudFBO", m_fragmentColorTextureClouds, 1);
    m_cloudPostProcessingShaderProgram->setVec2("resolution", glm::vec2(m_scene->getState()->getWidth(), m_scene->getState()->getHeight()));

    glDispatchCompute(m_width / 16, m_height / 16, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    
    retrieveSunColor();

    m_scene->getProfilerWindow()->EndGPUProfilerTask("clouds");
}

void Clouds::setCloudTexture(GLuint id)
{
    m_fragmentColorTextureGeometry = id;
}

void Clouds::drawGui()
{
    ImGui::Begin("Clouds");
//    ImGui::DragFloat3("Sun Position", &m_sunPosition.r);
    //ImGui::SliderFloat("Time of Day", &m_timeOfDay, 0.0f, 3600.0f);
    //Changed the range to 0 to 24, because a day has 24h -> more intuitive for the user
    ImGui::SliderFloat("Time of Day", &m_timeOfDay, 0.0f, 24.0f); 
    ImGui::Checkbox("Activate Time Laps", &m_timeLapsActivated);  
    ImGui::SliderFloat("Time speed", &m_timespeed, 0.001f, 0.1f);   
    ImGui::ColorEdit3("Background Color", &m_backgroundColor.r);
    ImGui::SliderFloat("Cloud Scale", &m_cloudScale, 10000.0f, 300000.0f);
    ImGui::SliderFloat("Earth Radius", &m_earthRadius, 100000.0f, 1000000.0f);
    ImGui::SliderFloat("Cloud Coverage Multiplier", &m_coverageFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Curliness", &m_curliness, 0.5f, 3.0f);
    ImGui::End();
}

GLuint Clouds::getCloudTexture()
{
    return m_fragmentColorTextureClouds;
}

void Clouds::update()
{
    // Update sun position
    float timeRadians = ((m_timeOfDay + 18) * 150) / 1800.0f * 3.141592653f;
    m_sunPosition = glm::vec3(-cos(timeRadians) * 10000.0f, sin(timeRadians) * 10000.0f, -10000.0f);

    drawGui();
}

void Clouds::initTextures()
{
    LOG_INFO("Generating noise textures for clouds...");
    if(!m_PerlinWorleyTexture)
    {
        // prepare texture for perlin-worley noise
        m_PerlinWorleyTexture = createTexture3D(128, 128, 128);
        m_PerlinWorleyNoiseComputeShaderProgram->use();
        bindTexture3D(m_PerlinWorleyTexture, 0);

        m_PerlinWorleyNoiseComputeShaderProgram->setInt("outVolTex", 0);

        // compute worley texture using the compute shader
        GLuint query;
        GLuint64 elapsed_time;

        // measure time using gl queries
        glGenQueries(1, &query);
        glBeginQuery(GL_TIME_ELAPSED, query);

        glDispatchCompute(32, 32, 32);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glEndQuery(GL_TIME_ELAPSED);

        // get the query result
        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
        LOG_INFO("Computed Perlin-Worley texture in " << static_cast<double>(elapsed_time / 1000000000.0) << " seconds.");


        // prepare texture for worley noise
        m_worleyTexture = createTexture3D(32, 32, 32);

        m_WorleyNoiseComputeShaderProgram->use();
        bindTexture3D(m_worleyTexture, 0);

        m_WorleyNoiseComputeShaderProgram->setInt("outVolTex", 0);

        glBeginQuery(GL_TIME_ELAPSED, query);

        glDispatchCompute(8, 8, 8);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glEndQuery(GL_TIME_ELAPSED);

        // get the query result
        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
        LOG_INFO("Computed Worley texture in " << static_cast<double>(elapsed_time / 1000000000.0) << " seconds.");
    }

    m_fragmentColorTextureClouds = createTexture2D(m_width, m_height);
    m_fragmentColorTextureCloudsBlured = createTexture2D(m_width, m_height);
}

void Clouds::initShaders()
{
    LOG_INFO("Initializing shaders for clouds");
    // textures needed for shader setup
    m_fragmentColorTextureClouds = createTexture2D(m_width, m_height);
    m_fragmentColorTextureCloudsBlured = createTexture2D(m_width, m_height);

    // create shader programs for perlin-worley generation and the volumetric cloud calculation
    m_PerlinWorleyNoiseComputeShaderProgram = std::make_unique<ShaderProgram>("perlinWorleyNoise");
    m_PerlinWorleyShader = std::make_shared<Shader>("./Assets/Shader/Clouds/PerlinWorleyNoise.comp");
    m_PerlinWorleyNoiseComputeShaderProgram->addShader(m_PerlinWorleyShader);
    m_PerlinWorleyNoiseComputeShaderProgram->link();

    // create shader programs for worley generation and the volumetric cloud calculation
    m_WorleyNoiseComputeShaderProgram = std::make_unique<ShaderProgram>("worleyNoise");
    m_WorleyShader = std::make_shared<Shader>("./Assets/Shader/Clouds/WorleyNoise.comp");
    m_WorleyNoiseComputeShaderProgram->addShader(m_WorleyShader);
    m_WorleyNoiseComputeShaderProgram->link();

    m_volumetricCloudsComputeShaderProgram = std::make_unique<ShaderProgram>("volumetricClouds");
    m_volumetricCloudsShader = std::make_shared<Shader>("./Assets/Shader/Clouds/Clouds.comp");
    m_volumetricCloudsComputeShaderProgram->addShader(m_volumetricCloudsShader);
    m_volumetricCloudsComputeShaderProgram->link();
    m_volumetricCloudsComputeShaderProgram->setSampler2D("fragColor", m_fragmentColorTextureCloudsBlured, 0);

    // setup cloud post processing shader to blur clouds
    m_cloudPostProcessingShaderProgram = std::make_unique<ShaderProgram>("cloudPostProcessing");
    m_cloudPostProcessingShader = std::make_shared<Shader>("./Assets/Shader/Clouds/CloudPostProcessing.comp");
    m_cloudPostProcessingShaderProgram->addShader(m_cloudPostProcessingShader);
    m_cloudPostProcessingShaderProgram->link();
}

glm::vec3* Clouds::getSunPosition()
{
    return &m_sunPosition;
}

glm::vec4* Clouds::getSunColor()
{
    return &m_sunColor;
}

void Clouds::initSunColorSSBO()
{
    // create sun color ssbo
    glCreateBuffers(1, &m_sunColorSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sunColorSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(float), nullptr, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                    7,
                    m_sunColorSSBO);
}

void Clouds::retrieveSunColor()
{
    GLsizeiptr sizeInBytes = 4 * sizeof(float);

    m_cloudPostProcessingShaderProgram->use();
    GLuint *buffer = &m_sunColorSSBO;

    // Map buffer for reading
    m_sunColor = *(glm::vec4*) 
        glMapNamedBuffer(*buffer, GL_READ_ONLY | GL_MAP_UNSYNCHRONIZED_BIT);
    glUnmapNamedBuffer(m_sunColorSSBO);

}
