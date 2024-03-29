//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "Defs.h"
#include "State.h"
#include "FBO.h"
#include "Objects/ScreenFillingQuad.h"
#include "Profiler.h"
#include <vector>
#include <variant>
#include <cctype>
#include <algorithm>
#include <tuple>

#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"
#include "Objects/Clouds.h"
#include "Objects/Ghost.h"
#include "Objects/GingerbreadHouse.h"
#include "Objects/Snowman.h"
#include "Objects/Stone1.h"
#include "Objects/Stone2.h"
#include "Objects/Stone3.h"
#include "Objects/Stone4.h"
#include "Objects/TreePlain.h"
#include "Objects/TreeGreen.h"
#include "Objects/TreeOlive.h"
#include "Objects/TreeGold.h"
#include "Objects/TreeBrown.h"

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
    void renderDrawables(std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>> drawables, std::shared_ptr<FBO> fbo);

    std::shared_ptr<State> getState() const;

    void addObject(std::shared_ptr<Drawable> object,
                   bool transparent = false,
                   Transformation transform = Transformation{glm::vec3(0.01f), glm::vec3(1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f});

    // Expose profiling window
    std::shared_ptr<ProfilersWindow> getProfilerWindow();

private:
    void drawGeometry();
    void drawTransparentGeometry();
    // Apply NPR effects to either the default FBO or the transparent object FBO
    void applyNPREffects(std::shared_ptr<FBO> fbo);
    // Final compositing pass
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


    std::shared_ptr<Drawable> getTreeColor();
    void loadMapTexture();
    void updateMapTexture();

    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<ProfilersWindow> m_profilerWindow;

    std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>> m_drawables;
    std::vector<std::pair<std::shared_ptr<Drawable>, Transformation>> m_transparentDrawables;
    std::shared_ptr<State> m_state;

    // SFQ for post processing
    std::shared_ptr<ScreenFillingQuad> m_sfq;

    // GBuffer
    std::shared_ptr<FBO> m_gBufferFBO;
    std::shared_ptr<FBO> m_gBufferTransparentFBO;

    // Sun position from clouds
    glm::vec3* m_sunPosition = nullptr;
    // Sun color from clouds
    glm::vec4* m_sunColor = nullptr;

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

    // General properties for all NPR effects
    float m_parallaxMappingHeightScale = 0.1f;
    bool m_useParallaxMapping = true;
    bool m_transparency = true;

    // Basic FBO drawing shader
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::shared_ptr<ShaderProgram> m_basicShaderProgram;

    // Gooch shader
    std::shared_ptr<Shader> m_goochVertexShader;
    std::shared_ptr<Shader> m_goochFragmentShader;
    std::shared_ptr<ShaderProgram> m_goochShaderProgram;
    // Gooch properties
    bool m_goochPropertyTextured = false;       // whether to use a texture or not
    bool m_goochPropertyUseSun = false;         // whether to use the sun as light source or not
    glm::vec3 m_goochPropertyCoolColor = glm::vec3(0.35, 0.45, 0.95);    //cold blue color
    glm::vec3 m_goochPropertyWarmColor = glm::vec3(1, 0.59, 0.4);        //warm orange color
    float m_goochPropertyDiffuseCool = 0.3f;    // scaling factor for the cool color
    float m_goochPropertyDiffuseWarm = 0.3f;    // scaling factor for the warm color

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
    bool m_JtoonPropertyTextured = false;         // whether to use a texture or not
    bool m_JtoonPropertyUseSun = false;           // whether to use the sun as light source or not
    int m_JtoonPropertyColorLevels = 5;           // adjustable number of different color levels
    float m_JtoonPropertyLevelBrightness = 0.4;   // parameter to brighten the result 
    float m_JtoonPropertySunlightInfluence = 0.4; // parameter to adjust how much the sun influences the color

    // Rim Lighting shader
    std::shared_ptr<Shader> m_rimLVertexShader;
    std::shared_ptr<Shader> m_rimLFragmentShader;
    std::shared_ptr<ShaderProgram> m_rimLShaderProgram;
    // Rim Lighting properties
    bool m_rimLPropertyTextured = false;  // wether to use a texture or not
    float m_rimLightStrength = 5.0f;

    // Alternative glow effect (by Jessica)
    std::shared_ptr<Shader> m_glowVertexShader;
    std::shared_ptr<Shader> m_glowFragmentShader;
    std::shared_ptr<ShaderProgram> m_glowShaderProgram;
    bool m_glowPropertyTextured = false;

    // PBR shader
    std::shared_ptr<Shader> m_pbrVertexShader;
    std::shared_ptr<Shader> m_pbrFragmentShader;
    std::shared_ptr<ShaderProgram> m_pbrShaderProgram;

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

    // Pattern shader
    std::shared_ptr<Shader> m_pattVertexShader;
    std::shared_ptr<Shader> m_pattFragmentShader;
    std::shared_ptr<ShaderProgram> m_pattShaderProgram;
    bool m_pattPropertyColored = false;          // whether to use object's colors or not
    bool m_pattPropertyTextured = false;         // whether to use the textures or not
    bool m_pattPropertyUseSun = false;           // whether to use the sun as a light source or not
    int m_pattPropertyMode = 0;
    float m_pattPropertyFrequency = 1.0;
    //bool m_pattPropertyNoiseActive = false;
    float m_pattPropertyNoiseFactor = 0.0;
    float m_pattPropertySunlightInfluence = 0.4;   // parameter to adjust how much the sun influences the color

    // Outline shader
    std::shared_ptr<Shader> m_outlVertexShader;
    std::shared_ptr<Shader> m_outlFragmentShader;
    std::shared_ptr<ShaderProgram> m_outlShaderProgram;
    bool m_OutlinePropertyColored = true;
    bool m_OutlinePropertyNormals = true;

    // Watercolor shader
    std::shared_ptr<Shader> m_waterColVertexShader;
    std::shared_ptr<Shader> m_waterColFragmentShader;
    std::shared_ptr<ShaderProgram> m_waterColShaderProgram;

    // Oil painting shader
    std::shared_ptr<Shader> m_oilVertexShader;
    std::shared_ptr<Shader> m_oilFragmentShader;
    std::shared_ptr<ShaderProgram> m_oilShaderProgram;
    int m_oilPropertyFilterRadius = 6;          // size of the filter kernel
    int m_oilPropertyNumberSubregions = 4;      // number of subregions
    bool m_oilPropertyUseSun = false;           // whether to use the sun as light source or not
        
    //////////////////////////////////////////
    // Objects
    std::shared_ptr<Drawable> m_triangle;
    std::shared_ptr<Drawable> m_terrain;
    std::shared_ptr<Drawable> m_clouds;
    std::shared_ptr<Drawable> m_ghost;
    std::shared_ptr<Drawable> m_gingerbreadHouse;
    std::shared_ptr<Drawable> m_snowMan;
    std::shared_ptr<Drawable> m_stone1;
    std::shared_ptr<Drawable> m_stone2;
    std::shared_ptr<Drawable> m_stone3;
    std::shared_ptr<Drawable> m_stone4;
    std::shared_ptr<Drawable> m_treePlain;
    std::shared_ptr<Drawable> m_treeGreen;
    std::shared_ptr<Drawable> m_treeOlive;
    std::shared_ptr<Drawable> m_treeGold;
    std::shared_ptr<Drawable> m_treeBrown;

    int m_season = 1;
    bool m_terrainPanMode = false;

};
