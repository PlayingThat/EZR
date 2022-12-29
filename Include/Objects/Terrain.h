//
// Created by jessb on 16.12.2022.
//

#pragma once

#include "../Engine/Shader.h"
#include "../Engine/ShaderProgram.h"
#include "../Engine/Drawable.h"
#include "../Engine/State.h"
#include "../Engine/Defs.h"

#include "../Engine/ConcurrentBinaryTree.h"
#include "../Engine/LongestEdgeBisection.h"

#include <map>
#include <string>


class Terrain : public Drawable, public SizeCallbackSubscriber
{
public:
    Terrain(std::shared_ptr<State> state);
    ~Terrain();

    void draw();

protected:
    void renderScene();
    void renderViewer();

    void onSizeChanged(int width, int height);

private:
    void drawGUI(); 

    std::shared_ptr<Terrain> m_terrain;
    
    std::shared_ptr<Shader> m_basicVertexShader;
    std::shared_ptr<Shader> m_basicFragmentShader;
    std::unique_ptr<ShaderProgram> m_basicShaderProgram;

    std::unique_ptr<ConcurrentBinaryTree> m_concurrentBinaryTree;
    std::unique_ptr<LongestEdgeBisection> m_longesEdgeBisection;

    // base buffers
    void create();

    // Setup buffers
    void setupBuffers();

    // Member functions to setup buffers
    void loadSubdivisionBuffer();
    // Buffers for indirect rendering of the terrain
    void loadRenderBuffer();
    // Buffers for the subdivided triangle, i.e. triangle meshlet
    void loadTriangleMeshletBuffers();

    void loadCBTNodeCountBuffer();

    //////////////////////////////////////////////////////////
    // GL Buffer

    // Subdivision buffer
    GLuint m_subdivisionBuffer = 0;
    
    // Buffer indices
    GLuint m_subdivionBufferIndex = 0;

    // Buffers for indirect drawing
    GLuint m_bufferTerrainDraw = 0;
    GLuint m_bufferTerrainDrawMeshTask = 0;
    GLuint m_bufferTerrainDrawComputeShader = 0;
    GLuint m_bufferTerrainDispatchComputeShader = 0;

    // Buffers for terrain subdivision meshlet
    GLuint m_bufferMeshletVertices = 0;
    GLuint m_bufferMeshletIndices = 0;

    GLuint m_bufferCBTNodeCount = 0;
    GLuint m_bufferCBTNodeCountIndex = 0;


    //////////////////////////////////////////////////////////
    // GUI Elements

    // Max subdivision depth of the terrain
    int m_maxDepth = 6;

    // Patch subdivision level to be sent to GPU
    int m_patchSubDiv = 0;

    //////////////////////////////////////////////////////////
    // Misc Engine Elements
    std::shared_ptr<State> m_state;

};