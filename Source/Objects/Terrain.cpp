//
// Created by jessb on 16.12.2022.
//

#include "Objects/Terrain.h"

Terrain::Terrain()
{
    create();
}

Terrain::~Terrain()
{
    //
}

void Terrain::draw()
{
    drawGUI();
}

// Draw GUI controls for terrain arguments
void Terrain::drawGUI()
{
    ImGui::Begin("Terrain");
    ImGui::SliderInt("Max Depth", &m_maxDepth, 1, 30);
    ImGui::End();
}

void Terrain::create()
{
/*     // Load shaders
    m_basicVertexShader = std::make_shared<Shader>("Shaders/basic.vert", GL_VERTEX_SHADER);
    m_basicFragmentShader = std::make_shared<Shader>("Shaders/basic.frag", GL_FRAGMENT_SHADER);

    // Create shader program
    m_basicShaderProgram = std::make_unique<ShaderProgram>();
    m_basicShaderProgram->attachShader(m_basicVertexShader);
    m_basicShaderProgram->attachShader(m_basicFragmentShader);
    m_basicShaderProgram->link(); */

    // Load subdivision buffer
    loadSubdivisionBuffer();
}

void Terrain::loadSubdivisionBuffer()
{
    m_concurrentBinaryTree = std::make_unique<ConcurrentBinaryTree>(m_maxDepth, 1);

    // Create a new shader storage buffer for the longest edge bisection
    glGenBuffers(1, &m_subdivisionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_subdivisionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_concurrentBinaryTree->heapByteSize(), m_concurrentBinaryTree->getHeap(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);

    if (glGetError() != GL_NO_ERROR)
    {
        LOG_ERROR("Error while loading subdivision buffer:" << glGetError());
    }
}