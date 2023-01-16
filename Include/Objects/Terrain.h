//
// Created by jessb on 16.12.2022.
//

#pragma once

#include "../Engine/Shader.h"
#include "../Engine/ShaderProgram.h"
#include "../Engine/Drawable.h"
#include "../Engine/State.h"
#include "../Engine/Scene.h"
#include "../Engine/Defs.h"
#include "../Engine/FBO.h"
#include "../Engine/Texture.h"

#include "../Engine/ConcurrentBinaryTree.h"
#include "../Engine/LongestEdgeBisection.h"

#include <map>
#include <string>

class Terrain : public Drawable, public SizeCallbackSubscriber
{
public:
    Terrain(std::shared_ptr<Scene> scene);
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

    //////////////////////////////////////////////////////////
    // Render functions
    void drawScene();

    void drawTerrain();

    void retrieveCBTNodeCount();


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

    // Setup framebuffers
    void loadTerrainFramebuffer();

    //////////////////////////////////////////////////////////
    // Setup vertex array objects
    void setupVAOs();
    void loadVAOEmpty();
    void loadVAOTriangleMeshlet();

    //////////////////////////////////////////////////////////
    // Load Queries
    void setupQueries();

    //////////////////////////////////////////////////////////
    // Shader programs
    void setupShaderPrograms();
    void loadShaderProgram(std::shared_ptr<ShaderProgram> &shaderProgram, std::string typeFlag);
    void loadTerrainPrograms();
    void loadLEBReductionProgram();
    void LoadLebReductionPrepassProgram();
    void loadBatchProgram();
    void loadTopViewProgram();
    void loadCBTNodeCountShader();

    // Configure shader programs
    void configureTerrainPrograms();
    void configureShaderProgram(std::shared_ptr<ShaderProgram> shaderProgram);
    void configureTerrainProgram();
    void configureTopViewProgram();
    

    //////////////////////////////////////////////////////////
    // GL Buffer

    // Subdivision buffer
    GLuint m_subdivisionBuffer = 0;
    
    // Buffer indices
    GLuint m_subdivionBufferIndex = 0;

    // Buffers for indirect drawing
    GLuint m_bufferTerrainDraw = 0;
    GLuint m_bufferTerrainDrawIndex = 0;
    GLuint m_bufferTerrainDrawComputeShader = 0;
    GLuint m_bufferTerrainDrawComputeShaderIndex = 0;
    GLuint m_bufferTerrainDispatchComputeShader = 0;
    GLuint m_bufferTerrainDispatchComputeShaderIndex = 0;

    // Buffers for terrain subdivision meshlet
    GLuint m_bufferMeshletVertices = 0;
    GLuint m_bufferMeshletIndices = 0;

    GLuint m_bufferCBTNodeCount = 0;
    GLuint m_bufferCBTNodeCountIndex = 0;

    // VAO for empty buffer
    GLuint m_vaoEmpty = 0;
    // VAO for triangle meshlet
    GLuint m_vaoTriangleMeshlet = 0;

    // Framebuffer for terrain
    GLuint m_framebufferTerrain = 0;
    GLuint m_framebufferTerrainColorTexture = 0;
    GLuint m_framebufferTerrainDepthTexture = 0;

    // Queries
    GLuint m_queryCBTNodeCount = 0;

    //////////////////////////////////////////////////////////
    // Shaders

    // Terrain main shaders
    std::shared_ptr<ShaderProgram> m_terrainMergeShaderProgram;
    std::shared_ptr<ShaderProgram> m_terrainSplitShaderProgram;
    std::shared_ptr<ShaderProgram> m_terrainDrawShaderProgram;

    std::shared_ptr<Shader> m_terrainShader;
    std::shared_ptr<Shader> m_terrainFrustumCullingShader;
    std::shared_ptr<Shader> m_terrainAtmosphereShader;
    std::shared_ptr<Shader> m_terrainLEBShader;
    std::shared_ptr<Shader> m_terrainCBTShader;
    std::shared_ptr<Shader> m_terrainRenderCommonShader;
    std::shared_ptr<Shader> m_terrainRenderSpecificShader;
    std::shared_ptr<Shader> m_terrainUpdateShader;

    // LEB reduction shaders
    std::shared_ptr<ShaderProgram> m_lebReductionShaderProgram;
    std::shared_ptr<Shader> m_cbtCumSumReductionShader;

    std::shared_ptr<ShaderProgram> m_lebReductionPerpassShaderProgram;
    std::shared_ptr<Shader> m_cbtCumSumReductionPrepassShader;

    std::shared_ptr<ShaderProgram> m_batchShaderProgram;
    std::shared_ptr<Shader> m_terrainBatchShader;

    std::shared_ptr<ShaderProgram> m_topViewShaderProgram;
    std::shared_ptr<Shader> m_topViewShader;

    std::shared_ptr<ShaderProgram> m_cbtNodeCountShaderProgram;
    std::shared_ptr<Shader> m_cbtNodeCountShader;


    //////////////////////////////////////////////////////////
    // Misc terrain variables
    uint32_t m_cbtNodeCount = 0;

    //////////////////////////////////////////////////////////
    // GUI Elements

    // Max subdivision depth of the terrain
    int m_maxDepth = 6;

    // Patch subdivision level to be sent to GPU
    int m_patchSubDiv = 0;

    //////////////////////////////////////////////////////////
    // Misc Engine Elements
    std::shared_ptr<Scene> m_scene;

};