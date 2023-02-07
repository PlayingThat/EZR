//
// Created by jesbu on 03.02.2023
//

#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Drawable.h"
#include "../Engine/Model.h"
#include "../Engine/Texture.h"

class TreeBrown : public Drawable
{
public:
    TreeBrown(std::shared_ptr<Scene> scene);

    void draw();

private:
    void create();
    
    std::unique_ptr<Model> m_model;

    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_textureFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    // Texture handle
    GLuint m_texture;
};

