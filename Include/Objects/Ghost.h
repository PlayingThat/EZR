//
// Created by maxbr on 29.12.2022.
//

#pragma once

#include "../Engine/Scene.h"
#include "../Engine/Drawable.h"
#include "../Engine/ModelLoader.h"
#include "../Engine/Texture.h"

class Ghost : public Drawable
{
public:
    Ghost(std::shared_ptr<Scene> scene);

    void draw();

private:
    void create();
    
    std::unique_ptr<ModelLoader> m_modelLoader;

    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_textureFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    // Texture handle
    GLUint m_texture;
};

