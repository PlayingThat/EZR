//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "Defs.h"
#include "State.h"
#include "FBO.h"
#include "Objects/ScreenFillingQuad.h"
#include <vector>

#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"
#include "Objects/Ghost.h"

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

    std::shared_ptr<Scene> m_scene;

    std::vector<std::shared_ptr<Drawable>> m_drawables;
    std::unique_ptr<float[]> m_backgroundColor;   
    std::shared_ptr<State> m_state;

    // SFQ for post processing
    std::shared_ptr<ScreenFillingQuad> m_sfq;

    // GBuffer
    std::unique_ptr<FBO> m_gBufferFBO;

    // GBuffer shader
    std::shared_ptr<Shader> m_gBufferVertexShader;
    std::shared_ptr<Shader> m_gBufferFragmentShader;
    std::unique_ptr<ShaderProgram> m_gBufferShaderProgram;

    // Objects
    std::shared_ptr<Drawable> m_triangle;
    std::shared_ptr<Drawable> m_terrain;
    std::shared_ptr<Drawable> m_ghost;



};