//
// Created by jessb on 16.12.2022.
//

#pragma once

#include "../Engine/Shader.h"
#include "../Engine/ShaderProgram.h"
#include "../Engine/Drawable.h"
#include "../Engine/State.h"
#include <string>

#include "../Engine/ConcurrentBinaryTree.h"
#include "../Engine/LongestEdgeBisection.h"

class Terrain : public Drawable
{
public:
    Terrain();
    ~Terrain();

    void draw();

    void loadSubdivisionBuffer();

private:
    void drawGUI(); 

    std::shared_ptr<Terrain> m_terrain;
    
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    std::unique_ptr<ConcurrentBinaryTree> m_concurrentBinaryTree;

    void create();

    //////////////////////////////////////////////////////////
    // GL Buffer

    // Subdivision buffer
    GLuint m_subdivisionBuffer = 0;
    
    // Buffer indices
    GLuint m_subdivionBufferIndex = 0;


    //////////////////////////////////////////////////////////
    // GUI Elements

    // Max subdivision depth of the terrain
    int m_maxDepth = 5;
};