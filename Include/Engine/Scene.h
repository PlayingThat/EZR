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

#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"
#include "Objects/Ghost.h"

// Struct for NPR properties
typedef struct
{
    std::string name;
    std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value;
    bool showInGUI = false;
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
    void addNPRProperty(std::string effectName, std::string propertyName, std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value, bool showInGUI = false);
    // Internal method to set the uniforms in the loop. don't call this!
    void setNPREffectProperty(std::shared_ptr<ShaderProgram> shaderProgram, std::string propertyName, std::variant<glm::vec2*, glm::vec3*, glm::vec4*, float*, int*, bool*, GLuint*> value);

    // Draw NPR property to GUI
    void drawNPREffectProperty(NPRProperty property);

    void drawNPRPanel();

    // Setup NPR effect shaders
    void setupNPREffects();

    std::shared_ptr<Scene> m_scene;

    std::vector<std::shared_ptr<Drawable>> m_drawables;
    std::unique_ptr<float[]> m_backgroundColor;   
    std::shared_ptr<State> m_state;

    // SFQ for post processing
    std::shared_ptr<ScreenFillingQuad> m_sfq;

    // GBuffer
    std::shared_ptr<FBO> m_gBufferFBO;

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

    // Toon shader
    std::shared_ptr<Shader> m_toonVertexShader;
    std::shared_ptr<Shader> m_toonFragmentShader;
    std::shared_ptr<ShaderProgram> m_toonShaderProgram;
    // Toon properties
    bool m_toonPropertyTextured = false;  // wether to use a texture or not


    //////////////////////////////////////////
    // Objects
    std::shared_ptr<Drawable> m_triangle;
    std::shared_ptr<Drawable> m_terrain;
    std::shared_ptr<Drawable> m_ghost;



};