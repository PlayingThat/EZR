//
// Created by jessb on 16.12.2022.
//

#include "Objects/Terrain.h"

Terrain::Terrain(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_scene = scene;
    create();

    // Subscribe to size change events
    m_scene->getState()->attachWindowSizeChangeCallback(this);
}

Terrain::~Terrain()
{
    //
}

void Terrain::draw()
{
    // drawGUI();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrain);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawScene();

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClearColor(0, 0, 0, 0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // renderViewer();
}

// Draw GUI controls for terrain arguments
void Terrain::drawGUI()
{
    ImGui::Begin("Terrain");
    ImGui::SliderInt("Max Depth", &m_maxDepth, 1, 30);
    ImGui::End();
}

// Callback for size change will need to recreate the buffers
void Terrain::onSizeChanged(int width, int height)
{
    setupBuffers();
    LOG_INFO("Size changed to: " + std::to_string(width) + "x" + std::to_string(height));
}

void Terrain::drawScene()
{
    drawTerrain();

}

void Terrain::drawTerrain()
{

}

void Terrain::retrieveCBTNodeCount()
{
    static GLint isReady = GL_FALSE;
    const GLuint *query = &m_queryCBTNodeCount;

    glGetQueryObjectiv(*query, GL_QUERY_RESULT_AVAILABLE, &isReady);

    if (isReady) {
        GLuint *buffer = &m_bufferCBTNodeCount;

        m_cbtNodeCount = *(uint32_t *)
            glMapNamedBuffer(*buffer, GL_READ_ONLY | GL_MAP_UNSYNCHRONIZED_BIT);
        glUnmapNamedBuffer(m_bufferCBTNodeCount);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         m_bufferCBTNodeCountIndex,
                         m_bufferCBTNodeCount);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         m_subdivionBufferIndex,
                         m_subdivisionBuffer);
        m_cbtNodeCountShaderProgram->use();
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glQueryCounter(*query, GL_TIMESTAMP);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         m_bufferCBTNodeCountIndex,
                         0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         m_subdivionBufferIndex,
                         0);
    }
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

    // Load buffers
    setupBuffers();

    // Load framebuffer
    loadTerrainFramebuffer();

    // Load Vertex attribute objects
    setupVAOs();
}

void Terrain::setupBuffers()
{
    // Clear any previous errors
    CLEAR_GL_ERRORS();

    // Load subdivision buffer
    loadSubdivisionBuffer();

    // Load buffers for indirect drawing
    loadRenderBuffer();

    loadTriangleMeshletBuffers();

    loadCBTNodeCountBuffer();

}

void Terrain::loadSubdivisionBuffer()
{
    LOG_INFO("Loading Subdivision buffer for LEB and CBT");

    m_concurrentBinaryTree = std::make_unique<ConcurrentBinaryTree>(m_maxDepth, 1);
    m_longesEdgeBisection = std::make_unique<LongestEdgeBisection>();

    // Create a new shader storage buffer for the longest edge bisection
    glGenBuffers(1, &m_subdivisionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_subdivisionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_concurrentBinaryTree->heapByteSize(), m_concurrentBinaryTree->getHeap(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);

    HANDLE_GL_ERRORS("loading terrain subdivision buffer (leb/cbt)");
}

void Terrain::loadRenderBuffer()
{
    uint32_t drawArraysCmd[8] = {2, 1, 0, 0, 0, 0, 0, 0};
    uint32_t drawMeshTasksCmd[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    uint32_t drawElementsCmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t dispatchCmd[8] = {2, 1, 1, 0, 0, 0, 0, 0};

    // Allow for dynamic parameter tuning via the GUI
    if (glIsBuffer(m_bufferTerrainDraw))
        glDeleteBuffers(1, &m_bufferTerrainDraw);

    if (glIsBuffer(m_bufferTerrainDrawMeshTask))
        glDeleteBuffers(1, &m_bufferTerrainDrawMeshTask);

    if (glIsBuffer(m_bufferTerrainDrawComputeShader))
        glDeleteBuffers(1, &m_bufferTerrainDrawComputeShader);

    glGenBuffers(1, &m_bufferTerrainDraw);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDraw);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawArraysCmd), drawArraysCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glGenBuffers(1, &m_bufferTerrainDrawMeshTask);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDrawMeshTask);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawMeshTasksCmd), drawMeshTasksCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glGenBuffers(1, &m_bufferTerrainDrawComputeShader);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDrawComputeShader);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawElementsCmd), drawElementsCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);


    glGenBuffers(1, &m_bufferTerrainDispatchComputeShader);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDispatchComputeShader);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(dispatchCmd), dispatchCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    HANDLE_GL_ERRORS("loading terrain render buffer");
}

void Terrain::loadCBTNodeCountBuffer()
{
    LOG_INFO("Loading Cbt-Node-Count-Buffer");
    if (glIsBuffer(m_bufferCBTNodeCount))
        glDeleteBuffers(1, &m_bufferCBTNodeCount);
    glGenBuffers(1, &m_bufferCBTNodeCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferCBTNodeCount);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                    sizeof(int32_t),
                    NULL,
                    GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     m_bufferCBTNodeCountIndex,
                     m_bufferCBTNodeCount);

    HANDLE_GL_ERRORS("loading CBT Node counter buffer");
}

void Terrain::loadTriangleMeshletBuffers()
{
    std::vector<uint16_t> indexBuffer;
    std::vector<glm::vec2> vertexBuffer;
    std::map<uint32_t, uint16_t> hashMap;
    int lebDepth = 2 * m_patchSubDiv;
    int triangleCount = 1 << lebDepth;
    int edgeTessellationFactor = 1 << m_patchSubDiv;

    // compute index and vertex buffer
    for (int i = 0; i < triangleCount; ++i) {
        cbt_Node node = {(uint64_t)(triangleCount + i), (uint64_t)2 * m_patchSubDiv};
        float attribArray[][3] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };

        m_longesEdgeBisection->decodeNodeAttributeArray(node, 2, attribArray);

        for (int j = 0; j < 3; ++j) {
            uint32_t vertexID = attribArray[0][j] * (edgeTessellationFactor + 1)
                              + attribArray[1][j] * (edgeTessellationFactor + 1) * (edgeTessellationFactor + 1);
            auto it = hashMap.find(vertexID);

            if (it != hashMap.end()) {
                indexBuffer.push_back(it->second);
            } else {
                uint16_t newIndex = (uint16_t)vertexBuffer.size();

                indexBuffer.push_back(newIndex);
                hashMap.insert(std::pair<uint32_t, uint16_t>(vertexID, newIndex));
                vertexBuffer.push_back(glm::vec2(attribArray[0][j], attribArray[1][j]));
            }
        }
    }

    if (glIsBuffer(m_bufferMeshletVertices))
        glDeleteBuffers(1, &m_bufferMeshletVertices);

    if (glIsBuffer(m_bufferMeshletIndices))
        glDeleteBuffers(1, &m_bufferMeshletIndices);

    LOG_INFO("Loading Meshlet-Buffers");

    glGenBuffers(1, &m_bufferMeshletIndices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferMeshletIndices);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(indexBuffer[0]) * indexBuffer.size(),
                 &indexBuffer[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_bufferMeshletVertices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferMeshletVertices);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(vertexBuffer[0]) * vertexBuffer.size(),
                 &vertexBuffer[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    HANDLE_GL_ERRORS("setting up meshlet buffers for LEB");
}

void Terrain::setupVAOs()
{
    loadVAOEmpty();
    loadVAOTriangleMeshlet();
}

void Terrain::loadVAOEmpty()
{
    LOG_INFO("loading Empty-VertexArray");
    if (glIsVertexArray(m_vaoEmpty))
        glDeleteVertexArrays(1, &m_vaoEmpty);

    glGenVertexArrays(1, &m_vaoEmpty);
    glBindVertexArray(m_vaoEmpty);
    glBindVertexArray(0);
    HANDLE_GL_ERRORS("loading VAO for empty terrain");
}

void Terrain::loadVAOTriangleMeshlet()
{
    LOG_INFO("loading VAO for triangle meshlet");
    if (glIsVertexArray(m_vaoTriangleMeshlet))
        glDeleteVertexArrays(1, &m_vaoTriangleMeshlet);

    glGenVertexArrays(1, &m_vaoTriangleMeshlet);

    glBindVertexArray(m_vaoTriangleMeshlet);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferMeshletVertices);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, ((char *)NULL + (0)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferMeshletIndices);
    glBindVertexArray(0);
    HANDLE_GL_ERRORS("loading VAO for triangle meshlet");
}

void Terrain::setupQueries()
{
    GLuint *query = &m_queryCBTNodeCount;

    glGenQueries(1, query);
    glQueryCounter(*query, GL_TIMESTAMP);

    HANDLE_GL_ERRORS("setting up terrain cbt node count queries");
}

void Terrain::setupShaderPrograms()
{
    loadLEBReductionProgram();
    LoadLebReductionPrepassProgram();
    loadBatchProgram();
    loadCBTNodeCountShader();
}

void Terrain::loadLEBReductionProgram()
{

}

void Terrain::LoadLebReductionPrepassProgram()
{

}

void Terrain::loadBatchProgram()
{

}

void Terrain::loadCBTNodeCountShader()
{

}

void Terrain::loadTerrainFramebuffer()
{
    LOG_INFO("loading terrain framebuffer");
    if (glIsFramebuffer(m_framebufferTerrain))
        glDeleteFramebuffers(1, &m_framebufferTerrain);

    glGenFramebuffers(1, &m_framebufferTerrain);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrain);

    GLuint *temp = createColorAttachments(m_scene->getState()->getCamera()->getWidth(),
                           m_scene->getState()->getCamera()->getHeight(),
                           2);
    m_framebufferTerrainColorTexture = temp[0];
    m_framebufferTerrainDepthTexture = temp[1];

    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_framebufferTerrainColorTexture,
        0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        m_framebufferTerrainDepthTexture,
        0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        LOG_ERROR("Error setting up terrain render framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
