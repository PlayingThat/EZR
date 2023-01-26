//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "Defs.h"
#include "State.h"
#include "FBO.h"
#include "Objects/ScreenFillingQuad.h"
#include <vector>
#include <variant>
#include <cctype>
#include <algorithm>

#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"
#include "Objects/Clouds.h"
#include "Objects/Ghost.h"

// Struct for NPR properties
typedef struct
{
    std::string name;
    std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value;
    bool showInGUI = false;
    float min = 0.0f;
    float max = 1.0f;
} NPRProperty;

// Struct for NPR effects
typedef struct 
{
    std::shared_ptr<ShaderProgram> shaderProgram;
    std::shared_ptr<FBO> fbo;
    std::vector<NPRProperty> properties;
    std::string name;
    bool enabled;
} NPREffect;

// Forward declaration
class Drawable;

class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene(std::shared_ptr<State> state);
    ~Scene();

    void setup(std::shared_ptr<Scene> scene);

    void update(float deltaTime);
    void renderDrawables();

    std::shared_ptr<State> getState() const;

    void addObject(std::shared_ptr<Drawable> object);

private:
    void drawGeometry();
    void drawSFQuad();

    void addNPREffect(std::shared_ptr<ShaderProgram> nprEffectProgram, bool enabledByDefault = false);

    // Add NPR property to NPR effect
    // effectName specifies the NPR effect to add the property to
    // propertyName specifies the name of the property, corresponding to the uniform name in the shader
    void addNPRProperty(std::string effectName, std::string propertyName, 
                        std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value, 
                        bool showInGUI = false,
                        float min = 0.0f,
                        float max = 1.0f);
    // Internal method to set the uniforms in the loop. don't call this!
    void setNPREffectProperty(std::shared_ptr<ShaderProgram> shaderProgram, std::string propertyName, std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value);

    // Draw NPR property to GUI
    void drawNPREffectProperty(NPRProperty property);

    void drawNPRPanel();

    // Setup NPR effect shaders
    void setupNPREffects();

    // Helpers
    std::string splitString(std::string s, char del);

    // Setup Stippling Textures
    void createStipplingTexture();

    std::shared_ptr<Scene> m_scene;

    std::vector<std::shared_ptr<Drawable>> m_drawables;
    std::unique_ptr<float[]> m_backgroundColor;   
    std::shared_ptr<State> m_state;

    // SFQ for post processing
    std::shared_ptr<ScreenFillingQuad> m_sfq;

    // GBuffer
    std::shared_ptr<FBO> m_gBufferFBO;

    // Terrain buffer textures
    GLuint* m_terrainTextures = nullptr;
    GLuint m_terrainTopView = 0;

    // NPR Shader effect FBOs
    // std::map<std::string, std::shared_ptr<FBO>> m_nprEffectFBOs;
    int m_enabledNPREffectCount = 0;

    // GBuffer shader
    std::shared_ptr<Shader> m_gBufferVertexShader;
    std::shared_ptr<Shader> m_gBufferFragmentShader;
    std::shared_ptr<ShaderProgram> m_gBufferShaderProgram;

    // Compositing shader
    std::shared_ptr<Shader> m_compositingVertexShader;
    std::shared_ptr<Shader> m_compositingFragmentShader;
    std::shared_ptr<ShaderProgram> m_compositingShaderProgram;
    GLuint m_cloudColorTexture = 0;

    // Vector of NP effect shader effects
    std::vector<std::shared_ptr<NPREffect>> m_NPREffects;
    // std::vector<std::string> m_nprEffectNames;

    //////////////////////////////////////////
    // NPR shader effects
    // Basic FBO drawing shader
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::shared_ptr<ShaderProgram> m_basicShaderProgram;

    // Gooch shader
    std::shared_ptr<Shader> m_goochVertexShader;
    std::shared_ptr<Shader> m_goochFragmentShader;
    std::shared_ptr<ShaderProgram> m_goochShaderProgram;
    // Gooch properties
    bool m_goochPropertyTextured = false;  // wether to use a texture or not
    glm::vec3 m_goochPropertyCoolColor = glm::vec3(0.35, 0.45, 0.95);    //cold blue color
    glm::vec3 m_goochPropertyWarmColor = glm::vec3(1, 0.59, 0.4);        //warm orange color

    // Toon shader (by Alyssa)
    std::shared_ptr<Shader> m_toonVertexShader;
    std::shared_ptr<Shader> m_toonFragmentShader;
    std::shared_ptr<ShaderProgram> m_toonShaderProgram;
    // Toon properties
    bool m_toonPropertyTextured = false;  // wether to use a texture or not

    // Toon shader (by Jessica)
    std::shared_ptr<Shader> m_JtoonVertexShader;
    std::shared_ptr<Shader> m_JtoonFragmentShader;
    std::shared_ptr<ShaderProgram> m_JtoonShaderProgram;
    // Toon properties
    bool m_JtoonPropertyTextured = false;  // wether to use a texture or not
    int m_JtoonPropertyColorLevels = 5;           // adjustable number of different color levels
    float m_JtoonPropertyLevelBrightness = 0.4;   // parameter to brighten the result 

    // Rim Lighting shader
    std::shared_ptr<Shader> m_rimLVertexShader;
    std::shared_ptr<Shader> m_rimLFragmentShader;
    std::shared_ptr<ShaderProgram> m_rimLShaderProgram;
    // Rim Lighting properties
    bool m_rimLPropertyTextured = false;  // wether to use a texture or not

    // Stippling Textures
    GLuint m_stipp1 = 0;
    GLuint m_stipp2 = 0;
    GLuint m_stipp3 = 0;
    GLuint m_stipp4 = 0;
    GLuint m_stipp5 = 0;
    GLuint m_stipp6 = 0;
    GLuint m_paper = 0;
    GLuint m_noise = 0;
    GLuint m_canvas = 0;

    // Hatching shader
    std::shared_ptr<Shader> m_hatchVertexShader;
    std::shared_ptr<Shader> m_hatchFragmentShader;
    std::shared_ptr<ShaderProgram> m_hatchShaderProgram;
    bool m_hatchPropertyColored = false;
    bool m_hatchPropertyTextured = false;
    int m_hatchPropertyMode = 0;
    float m_hatchPropertyFrequency = 1.0;
    bool m_hatchPropertyNoiseActive = false;
    float m_hatchPropertyNoise = 3.0;

    // Outline shader
    std::shared_ptr<Shader> m_outlVertexShader;
    std::shared_ptr<Shader> m_outlFragmentShader;
    std::shared_ptr<ShaderProgram> m_outlShaderProgram;

    // Watercolor shader
    std::shared_ptr<Shader> m_waterColVertexShader;
    std::shared_ptr<Shader> m_waterColFragmentShader;
    std::shared_ptr<ShaderProgram> m_waterColShaderProgram;
    
    //////////////////////////////////////////
    // Objects
    std::shared_ptr<Drawable> m_triangle;
    std::shared_ptr<Drawable> m_terrain;
    std::shared_ptr<Drawable> m_clouds;
    std::shared_ptr<Drawable> m_ghost;



};
