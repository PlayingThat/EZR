//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "../Engine/Shader.h"
#include "../Engine/ShaderProgram.h"
#include "../Engine/Drawable.h"
#include "../Engine/Scene.h"
#include "../Objects/Triangle.h"

class ColorfullTriangle : public Drawable
{
public:
    ColorfullTriangle(std::shared_ptr<Scene> scene);
    ~ColorfullTriangle();

    void draw();

private:
    std::shared_ptr<Triangle> m_triangle;
    
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    std::unique_ptr<float[]> m_color;
};