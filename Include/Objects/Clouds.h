//
// Created by maxbr on 10.01.2023.
//

#pragma once

#include "../Engine/Defs.h"
#include "../Engine/Drawable.h"
#include "../Engine/Scene.h"
#include "../Engine/ShaderProgram.h"

class Clouds : public Drawable
{
public:
    Clouds(std::shared_ptr<Scene> scene);

    void draw() override;

    void setCloudTexture(GLuint id);

    GLuint getCloudTexture();

    virtual void update();


private:
    void initTextures();

    void initShaders();

    void drawGui();

    std::shared_ptr<Scene> m_scene;
    GLuint m_fragmentColorTextureGeometry;
    GLuint m_fragmentColorTextureClouds;
    GLuint m_fragmentColorTextureCloudsBlured;

    std::shared_ptr<Shader> m_volumetricCloudsShader;
    std::shared_ptr<Shader> m_PerlinWorleyShader;
    std::shared_ptr<Shader> m_WorleyShader;
    std::shared_ptr<Shader> m_cloudPostProcessingShader;

    std::unique_ptr<ShaderProgram> m_PerlinWorleyNoiseComputeShaderProgram;
    std::unique_ptr<ShaderProgram> m_WorleyNoiseComputeShaderProgram;
    std::unique_ptr<ShaderProgram> m_volumetricCloudsComputeShaderProgram;
    std::unique_ptr<ShaderProgram> m_cloudPostProcessingShaderProgram;
    GLuint m_PerlinWorleyTexture;
    GLuint m_worleyTexture;

    size_t m_width, m_height;

    // scene properties
    glm::vec3 m_sunPosition;
    glm::vec3 m_sunColor;
    glm::vec3 m_ambientColor;

    glm::vec3 m_backgroundColor;
    glm::vec3 m_dayColor;
    glm::vec3 m_sunsetColor;
    glm::vec3 m_nightColor;

    float m_timeOfDay = 1800.0f;  // 12pm
    float m_windSpeed = 300.0f;

    float m_earthRadius;
    float m_coverageFactor;
    float m_curliness;

    float m_cloudScale;
};
