//
// Created by jessb on 16.12.2022.
//

#pragma once

#include "../Engine/Shader.h"
#include "../Engine/ShaderProgram.h"
#include "../Engine/Drawable.h"
#include "../Engine/State.h"
#include <string>

class Terrain : public Drawable
{
public:
    Terrain();
    ~Terrain();

    void draw();

private:
    std::shared_ptr<Terrain> m_terrain;
    
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    void create();
};