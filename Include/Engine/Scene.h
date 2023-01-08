//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "Defs.h"
#include "State.h"
#include "FBO.h"
#include "Objects/ScreenFillingQuad.h"
#include <vector>
#include <map>

#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"
#include "Objects/Ghost.h"

// Struct for NPR effects
typedef struct 
{
    std::shared_ptr<ShaderProgram> shaderProgram;
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

    void drawNPRDragAndDrop();
    void recalculateNPRDragAndDrop();

    // Setup NPR effect shaders
    void setupNPREffects();

    // Setup NPR effect FBOs
    void setupNPRFBOs();


    std::shared_ptr<Scene> m_scene;

    std::vector<std::shared_ptr<Drawable>> m_drawables;
    std::unique_ptr<float[]> m_backgroundColor;   
    std::shared_ptr<State> m_state;

    // SFQ for post processing
    std::shared_ptr<ScreenFillingQuad> m_sfq;

    // GBuffer
    std::shared_ptr<FBO> m_gBufferFBO;

    // NPR Shader effect FBOs
    std::map<std::string, std::shared_ptr<FBO>> m_nprEffectFBOs;
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
    std::vector<std::string> m_nprEffectNames;

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


    //////////////////////////////////////////
    // Objects
    std::shared_ptr<Drawable> m_triangle;
    std::shared_ptr<Drawable> m_terrain;
    std::shared_ptr<Drawable> m_ghost;



};