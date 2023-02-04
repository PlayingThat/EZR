//
// Created by jessb on 16.12.2022.
//

#include "Objects/Terrain.h"
#define CBT_IMPLEMENTATION
#include "../../Include/Engine/cbt.h"
#define LEB_IMPLEMENTATION
#include "../../Include/Engine/leb.h"

Terrain::Terrain(std::shared_ptr<Scene> scene) : Drawable(scene)
{
    m_scene = scene;

    // Setup transform
    rotate(glm::vec3(1, 0, 0), -90.0f);
    scale(glm::vec3(1000, 30, 1000));
    // setBasePosition(glm::vec3(-50, -2, 15));

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
    m_scene->getProfilerWindow()->StartGPUProfilerTask("terrain");
    drawGUI();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrain);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrainTopViewFBO->getID());
    glViewport(10, 10, 300, 300);
    glClearColor(0, 0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawTopView();
    
    m_scene->getProfilerWindow()->EndGPUProfilerTask("terrain");
}

// Draw GUI controls for terrain arguments
void Terrain::drawGUI()
{
    ImGui::Begin("Terrain");
    ImGui::Text("CBT Node Count: %i", m_cbtNodeCount);
    if (ImGui::SliderInt("Max Depth", &m_maxDepth, 6, 25)) {
        setupBuffers();
        setupShaderPrograms();
    }
    if (ImGui::SliderInt("Patch Sub Level", &m_patchSubDiv, 1, 10)) {
        loadSubdivisionBuffer();
        loadVAOTriangleMeshlet();
        setupShaderPrograms();
    }
    ImGui::SliderFloat("Height Scale", &m_dmapFactor, 0.0f, 1.0f);
    ImGui::Checkbox("Wireframe", &m_wireframe);
    ImGui::End();
}

// Callback for size change will need to recreate the buffers
void Terrain::onSizeChanged(int width, int height)
{
    loadSceneFramebufferTexture();
    loadTerrainFramebuffer();
    configureTerrainPrograms();
    configureTopViewProgram();

    LOG_INFO("Size changed to: " + std::to_string(width) + "x" + std::to_string(height));
}

void Terrain::drawScene()
{
    drawTerrain();
    retrieveCBTNodeCount();
    renderSky();

}

void Terrain::drawTopView()
{
    // glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrainTopViewFBO->getID());
    m_topViewShaderProgram->use();
    configureTopViewProgram();
    glBindVertexArray(m_vaoEmpty);

    glDisable(GL_CULL_FACE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDraw);
    glPatchParameteri(GL_PATCH_VERTICES, 1);

    glDrawArraysIndirect(GL_PATCHES, 0);

    // reset GL state
    glBindVertexArray(0);
    glViewport(0, 0, m_scene->getState()->getWidth(), m_scene->getState()->getHeight());
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Terrain::renderSky()
{
    // glDepthMask(GL_FALSE);
    // m_skybox->draw();
    // glDepthMask(GL_TRUE);
}

void Terrain::drawTerrain()
{
    configureTerrainPrograms();
    loadTerrainVariables();
    lebUpdate();
    lebReductionPass();
    lebBatchingPass();
    lebRender();
}

void Terrain::lebUpdate()
{
    static int pingPong = 0;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);

    // set GL state
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, m_bufferTerrainDispatchComputeShader);

    // update
    if (pingPong == 0)
    {
        m_terrainSplitShaderProgram->use();
    }
    else
    {
        m_terrainMergeShaderProgram->use();
    }
    glDispatchComputeIndirect(0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reset GL state
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, 0);
    pingPong = 1 - pingPong;
}

void Terrain::lebReductionPass()
{
    int it = m_maxDepth;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);
    m_lebReductionPrepassShaderProgram->use();
    if (true) {
        int cnt = ((1 << it) >> 5);// / 2;
        int numGroup = (cnt >= 256) ? (cnt >> 8) : 1;
        
        m_lebReductionPrepassShaderProgram->setInt("u_PassID", it);
        glDispatchCompute(numGroup, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        it-= 5;
    }

    m_lebReductionShaderProgram->use();
    while (--it >= 0) {
        int cnt = 1 << it;
        int numGroup = (cnt >= 256) ? (cnt >> 8) : 1;

        m_lebReductionShaderProgram->setInt("u_PassID", it);
        glDispatchCompute(numGroup, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, 0);
}

void Terrain::lebBatchingPass()
{
    m_batchShaderProgram->use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     m_bufferTerrainDrawIndex,
                     m_bufferTerrainDraw);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     m_bufferTerrainDrawComputeShaderIndex,
                     m_bufferTerrainDrawComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     m_bufferTerrainDispatchComputeShaderIndex,
                     m_bufferTerrainDispatchComputeShader);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bufferTerrainDrawIndex, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bufferTerrainDrawComputeShaderIndex, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bufferTerrainDispatchComputeShaderIndex, 0);
}

void Terrain::lebRender()
{
    if (m_wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // set GL state
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDrawComputeShader);
    glBindVertexArray(m_vaoTriangleMeshlet);

    // render
    m_terrainDrawShaderProgram->use();
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, ((char *)NULL + (0)));  // 0 = offset

    // reset GL state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
}

void Terrain::create()
{
    LOG_INFO("Try to create Terrain");

    // Load textures
    loadTextures();

    // Load buffers
    setupBuffers();

    // Load framebuffer
    loadTerrainFramebuffer();

    // Load Vertex attribute objects
    setupVAOs();

    // Load shaders
    setupShaderPrograms();

    // Load queries
    setupQueries();

    LOG_INFO("Terrain created");
}

void Terrain::setupBuffers()
{
    // Set stream buffer variables
    loadTerrainVariables();

    // Load subdivision buffer
    loadSubdivisionBuffer();

    // Load buffers for indirect drawing
    loadRenderBuffer();

    loadTriangleMeshletBuffers();

    loadCBTNodeCountBuffer();

    static bool first = true;
    // Setup Top view render buffer
    if (first) {
        first = false;
        m_framebufferTerrainTopViewFBO = std::make_shared<FBO>(m_scene, 1);
    }

}

void Terrain::setTerrainVariables(std::shared_ptr<ShaderProgram> &shaderProgram,
                                  glm::mat4* modelMatrix,
                                  glm::mat4* viewMatrix,
                                  glm::mat4* projectionMatrix,
                                  glm::vec4 frustum[6])
{
    shaderProgram->use();
    shaderProgram->setMat4("u_ModelMatrix", glm::transpose(*modelMatrix));
    shaderProgram->setMat4("u_ModelViewMatrix", glm::transpose(*viewMatrix * *modelMatrix));
    shaderProgram->setMat4("u_ViewMatrix", glm::transpose(*viewMatrix));
    shaderProgram->setMat4("u_CameraMatrix", glm::transpose(glm::inverse(*viewMatrix)));
    shaderProgram->setMat4("u_ViewProjectionMatrix", glm::transpose(*projectionMatrix * *viewMatrix));
    shaderProgram->setMat4("u_ModelViewProjectionMatrix", *projectionMatrix * *viewMatrix * *modelMatrix);
    shaderProgram->setVec4("u_FrustumPlanes", *frustum, 6);
}

void Terrain::loadTerrainVariables()
{
    static bool first = true;
    struct PerFrameVariables {
        glm::mat4 model,                // 16
                  modelView,            // 16
                  view,                 // 16
                  camera,               // 16
                  viewProjection,       // 16
                  modelViewProjection;  // 16
        glm::vec4 frustum[6];           // 24
        glm::vec4 align[2];             // 8
    } variables;

    if (first) {
        first = false;    
        const int buf_capacity = (1 << 20); // capacity in Bytes
        StreamBuffer *buffer = (StreamBuffer*)malloc(sizeof(*buffer));
        GLint buf = 0;

        buffer->capacity = buf_capacity;
        buffer->size = sizeof(variables);
        buffer->offset = 0;

        glGenBuffers(1, &buffer->gl);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buf);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->gl);
        glBufferData(GL_ARRAY_BUFFER, buffer->capacity, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, buf);

        m_streamTerrainVariables = buffer;
    }

    setBasePosition(glm::vec3(-50, -2, 15));
    glm::mat4 modelMatrix = getModelMatrix();
    glm::mat4 viewMatrix = *m_scene->getState()->getCamera()->getViewMatrix();
    glm::mat4 projectionMatrix = *m_scene->getState()->getCamera()->getProjectionMatrix();

    variables.model = modelMatrix;
    variables.modelView = viewMatrix * modelMatrix;
    variables.view = viewMatrix;
    variables.camera = glm::transpose(glm::inverse(viewMatrix));
    variables.viewProjection = projectionMatrix * viewMatrix;
    variables.modelViewProjection = projectionMatrix * viewMatrix * modelMatrix;  

    // extract frustum planes from modelViewProjection matrix
    glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 2; ++j) {
        variables.frustum[i*2+j].x = mvp[0][3] + (j == 0 ? mvp[0][i] : -mvp[0][i]);
        variables.frustum[i*2+j].y = mvp[1][3] + (j == 0 ? mvp[1][i] : -mvp[1][i]);
        variables.frustum[i*2+j].z = mvp[2][3] + (j == 0 ? mvp[2][i] : -mvp[2][i]);
        variables.frustum[i*2+j].w = mvp[3][3] + (j == 0 ? mvp[3][i] : -mvp[3][i]);
        glm::vec4 tmp = variables.frustum[i*2+j];
        // norm of vector
        variables.frustum[i*2+j] *= glm::sqrt(glm::dot(glm::vec3(tmp), glm::vec3(tmp)));
    }

    // for (int i = 4; i--; ) variables.frustum[0][i]   = mvp[i][3] + mvp[i][0];
    // for (int i = 4; i--; ) variables.frustum[1][i]  = mvp[i][3] - mvp[i][0];
    // for (int i = 4; i--; ) variables.frustum[2][i] = mvp[i][3] + mvp[i][1];
    // for (int i = 4; i--; ) variables.frustum[3][i]    = mvp[i][3] - mvp[i][1];
    // for (int i = 4; i--; ) variables.frustum[4][i]   = mvp[i][3] + mvp[i][2];
    // for (int i = 4; i--; ) variables.frustum[5][i]    = mvp[i][3] - mvp[i][2];

    if (!bufferToGL(m_streamTerrainVariables, (const void *)&variables, NULL))
        LOG_ERROR("bufferToGL failed");
    
    int tmp = m_streamTerrainVariables->offset + m_streamTerrainVariables->size * (0 - 1);
    int maxOffset = m_streamTerrainVariables->capacity;
    int bufOffset = ((tmp % maxOffset) + maxOffset) % maxOffset;

    glBindBufferRange(GL_UNIFORM_BUFFER, m_streamTerrainVariablesIndex, m_streamTerrainVariables->gl, bufOffset, m_streamTerrainVariables->size);
    
    HANDLE_GL_ERRORS("setting terrain matrix uniforms");
}

void Terrain::loadTextures()
{
    loadSceneFramebufferTexture();
    loadTerrainMaps("./Assets/Terrain/displacement_map_kauai.png");
    loadAtmosphereTexture();
}

void Terrain::loadAtmosphereTexture()
{
    LOG_INFO("Loading atmosphere textures");
    float *data = new float[16*64*3];
    FILE *f = fopen("./Assets/Terrain/irradiance.raw", "rb");
    if (!f) {
        LOG_ERROR("Failed to open terrain atmosphere texture");
    }
    fread(data, 1, 16*64*3*sizeof(float), f);
    fclose(f);
    glGenTextures(1, &m_irradianceTexture);
    glActiveTexture(GL_TEXTURE0 + m_irradianceTexture);
    glBindTexture(GL_TEXTURE_2D, m_irradianceTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 64, 16, 0, GL_RGB, GL_FLOAT, data);
    delete[] data;

    int res = 64;
    int nr = res / 2;
    int nv = res * 2;
    int nb = res / 2;
    int na = 8;
    f = fopen("./Assets/Terrain/inscatter.raw", "rb");
    if (!f) {
        LOG_ERROR("Failed to open terrain inscatter texture");
    }
    data = new float[nr*nv*nb*na*4];
    fread(data, 1, nr*nv*nb*na*4*sizeof(float), f);
    fclose(f);
    glGenTextures(1, &m_inscatterTexture);
    glActiveTexture(GL_TEXTURE0 + m_inscatterTexture);
    glBindTexture(GL_TEXTURE_3D, m_inscatterTexture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, na*nb, nv, nr, 0, GL_RGBA, GL_FLOAT, data);
    delete[] data;

    data = new float[256*64*3];
    f = fopen("./Assets/Terrain/transmittance.raw", "rb");
    if (!f) {
        LOG_ERROR("Failed to open terrain transmittance texture");
    }
    fread(data, 1, 256*64*3*sizeof(float), f);
    fclose(f);
    glGenTextures(1, &m_transmittanceTexture);
    glActiveTexture(GL_TEXTURE0 + m_transmittanceTexture);
    glBindTexture(GL_TEXTURE_2D, m_transmittanceTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 256, 64, 0, GL_RGB, GL_FLOAT, data);
    delete[] data;
    HANDLE_GL_ERRORS("loading terrain atmosphere textures");
}

void Terrain::loadSceneFramebufferTexture()
{
    if(glIsTexture(m_framebufferTerrainDepthTexture))
        glDeleteTextures(1, &m_framebufferTerrainDepthTexture);
    if(glIsTexture(m_framebufferTerrainColorTexture))
        glDeleteTextures(1, &m_framebufferTerrainColorTexture);

    glGenTextures(1, &m_framebufferTerrainDepthTexture);
    glActiveTexture(GL_TEXTURE0 + m_framebufferTerrainDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTerrainDepthTexture);
    glTexStorage2D(GL_TEXTURE_2D,
        1,
        GL_DEPTH24_STENCIL8,
        m_scene->getState()->getCamera()->getWidth(),
        m_scene->getState()->getCamera()->getHeight());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &m_framebufferTerrainColorTexture);
    glActiveTexture(GL_TEXTURE0 + m_framebufferTerrainColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTerrainColorTexture);
    
    glTexStorage2D(GL_TEXTURE_2D,
        1,
        GL_RGBA32F,
        m_scene->getState()->getCamera()->getWidth(),
        m_scene->getState()->getCamera()->getHeight());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);

    HANDLE_GL_ERRORS("setting up terrain frame buffer textures");
}

void Terrain::loadTerrainMaps(std::string filePath)
{
    LOG_INFO("Loading displacement map for terrain");

    m_slopeTerrainMapPixels = createTextureFromFile16(filePath.c_str(), m_slopeTerrainMapWidth, m_slopeTerrainMapHeight);

    // const uint16_t *texels = (const uint16_t *)djgt->next->texels;
    std::vector<uint16_t> dmap(m_slopeTerrainMapWidth * m_slopeTerrainMapHeight * 2);

    for (int j = 0; j < m_slopeTerrainMapHeight; ++j)
    {
        for (int i = 0; i < m_slopeTerrainMapWidth; ++i) 
        {
            uint16_t z = m_slopeTerrainMapPixels[i + m_slopeTerrainMapWidth * j]; // in [0,2^16-1]
            float zf = float(z) / float((1 << 16) - 1);
            uint16_t z2 = zf * zf * ((1 << 16) - 1);

            dmap[    2 * (i + m_slopeTerrainMapWidth * j)] = z;
            dmap[1 + 2 * (i + m_slopeTerrainMapWidth * j)] = z2;
        }
    }

    // Generate slope map from displacement map
    generateSlopeMap();

    if (glIsTexture(m_displacementMapTexture))
        glDeleteTextures(1, &m_displacementMapTexture);

    glGenTextures(1, &m_displacementMapTexture);
    glActiveTexture(GL_TEXTURE0 + m_displacementMapTexture);
    glBindTexture(GL_TEXTURE_2D, m_displacementMapTexture);
    glTexStorage2D(GL_TEXTURE_2D, (int)glm::log2((float)glm::max(m_slopeTerrainMapWidth, m_slopeTerrainMapHeight)),
                   GL_RG16, m_slopeTerrainMapWidth, m_slopeTerrainMapHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_slopeTerrainMapWidth, m_slopeTerrainMapHeight, GL_RG, GL_UNSIGNED_SHORT, &dmap[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
    delete[] m_slopeTerrainMapPixels;

    HANDLE_GL_ERRORS("loading terrain displacement map");
}

void Terrain::generateSlopeMap()
{
    LOG_INFO("Generating slope map for terrain");
    int w = m_slopeTerrainMapWidth;
    int h = m_slopeTerrainMapHeight;

    std::vector<float> smap(w * h * 2);

    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i) {
            int i1 = std::max(0, i - 1);
            int i2 = std::min(w - 1, i + 1);
            int j1 = std::max(0, j - 1);
            int j2 = std::min(h - 1, j + 1);
            uint16_t px_l = m_slopeTerrainMapPixels[i1 + w * j]; // in [0,2^16-1]
            uint16_t px_r = m_slopeTerrainMapPixels[i2 + w * j]; // in [0,2^16-1]
            uint16_t px_b = m_slopeTerrainMapPixels[i + w * j1]; // in [0,2^16-1]
            uint16_t px_t = m_slopeTerrainMapPixels[i + w * j2]; // in [0,2^16-1]
            float z_l = (float)px_l / 65535.0f; // in [0, 1]
            float z_r = (float)px_r / 65535.0f; // in [0, 1]
            float z_b = (float)px_b / 65535.0f; // in [0, 1]
            float z_t = (float)px_t / 65535.0f; // in [0, 1]
            float slope_x = (float)w * 0.5f * (z_r - z_l);
            float slope_y = (float)h * 0.5f * (z_t - z_b);

            smap[    2 * (i + w * j)] = slope_x;
            smap[1 + 2 * (i + w * j)] = slope_y;
        }

    glGenTextures(1, &m_slopeMapTexture);
    glActiveTexture(GL_TEXTURE0 + m_slopeMapTexture);
    glBindTexture(GL_TEXTURE_2D, m_slopeMapTexture);
    glTexStorage2D(GL_TEXTURE_2D, (int)glm::log2((float)glm::max(w, h)), GL_RG32F, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RG, GL_FLOAT, &smap[0]);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);

    HANDLE_GL_ERRORS("generating slope map");
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

void Terrain::loadSubdivisionBuffer()
{
    LOG_INFO("Loading Subdivision buffer for LEB and CBT");

    // m_concurrentBinaryTree = std::make_unique<ConcurrentBinaryTree>(m_maxDepth, 1);
    // m_longesEdgeBisection = std::make_unique<LongestEdgeBisection>();

    cbt_Tree *cbt = cbt_CreateAtDepth(m_maxDepth, 1);
    
    if (glIsBuffer(m_subdivisionBuffer))
        glDeleteBuffers(1, &m_subdivisionBuffer);
    // Create a new shader storage buffer for the longest edge bisection
    glCreateBuffers(1, &m_subdivisionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_subdivisionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 
                 cbt_HeapByteSize(cbt),
                 cbt_GetHeap(cbt),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_subdivionBufferIndex, m_subdivisionBuffer);
    cbt_Release(cbt);
    HANDLE_GL_ERRORS("loading terrain subdivision buffer (leb/cbt)");
}

void Terrain::loadRenderBuffer()
{
    uint32_t drawArraysCmd[8] = {2, 1, 0, 0, 0, 0, 0, 0};
    uint32_t drawElementsCmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t dispatchCmd[8] = {2, 1, 1, 0, 0, 0, 0, 0};

    // Allow for dynamic parameter tuning via the GUI
    if (glIsBuffer(m_bufferTerrainDraw))
        glDeleteBuffers(1, &m_bufferTerrainDraw);

    if (glIsBuffer(m_bufferTerrainDrawComputeShader))
        glDeleteBuffers(1, &m_bufferTerrainDrawComputeShader);

    glCreateBuffers(1, &m_bufferTerrainDraw);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDraw);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawArraysCmd), drawArraysCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glCreateBuffers(1, &m_bufferTerrainDrawComputeShader);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufferTerrainDrawComputeShader);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawElementsCmd), drawElementsCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glCreateBuffers(1, &m_bufferTerrainDispatchComputeShader);
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
    glCreateBuffers(1, &m_bufferCBTNodeCount);
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

        leb_DecodeNodeAttributeArray(node, 2, attribArray);

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

    glCreateBuffers(1, &m_bufferMeshletIndices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferMeshletIndices);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(indexBuffer[0]) * indexBuffer.size(),
                 &indexBuffer[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glCreateBuffers(1, &m_bufferMeshletVertices);
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
    LOG_INFO("Loading Empty-VertexArray");
    if (glIsVertexArray(m_vaoEmpty))
        glDeleteVertexArrays(1, &m_vaoEmpty);

    glGenVertexArrays(1, &m_vaoEmpty);
    glBindVertexArray(m_vaoEmpty);
    glBindVertexArray(0);
    HANDLE_GL_ERRORS("loading VAO for empty terrain");
}

void Terrain::loadVAOTriangleMeshlet()
{
    LOG_INFO("Loading VAO for triangle meshlet");
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
    glGenQueries(1, &m_queryCBTNodeCount);
    glQueryCounter(m_queryCBTNodeCount, GL_TIMESTAMP);

    HANDLE_GL_ERRORS("setting up terrain cbt node count queries");
}

void Terrain::setupShaderPrograms()
{
    loadTerrainPrograms();
    loadLEBReductionProgram();
    LoadLebReductionPrepassProgram();
    loadBatchProgram();
    loadTopViewProgram();
    loadCBTNodeCountShader();
}

void Terrain::loadShaderProgram(std::shared_ptr<ShaderProgram> &shaderProgram, std::string typeFlag)
{
    LOG_INFO("Loading terrain " + typeFlag +" shader program");
    
    shaderProgram = std::make_shared<ShaderProgram>(typeFlag);
    shaderProgram->addSource("#define PROJECTION_RECTILINEAR");

    shaderProgram->addSource("#define BUFFER_BINDING_MESHLET_VERTICES " + std::to_string(m_bufferMeshletVertices));
    shaderProgram->addSource("#define BUFFER_BINDING_MESHLET_INDEXES " + std::to_string(m_bufferMeshletIndices));
    shaderProgram->addSource("#define TERRAIN_PATCH_SUBD_LEVEL " + std::to_string(m_patchSubDiv));
    shaderProgram->addSource("#define TERRAIN_PATCH_TESS_FACTOR " + std::to_string((1 << m_patchSubDiv)));
    shaderProgram->addSource("#define BUFFER_BINDING_TERRAIN_VARIABLES " + std::to_string(m_streamTerrainVariablesIndex));
    shaderProgram->addSource("#define SHADING_DIFFUSE 1");
    shaderProgram->addSource("#define FLAG_DISPLACE 1");
    shaderProgram->addSource("#define FLAG_CULL 1");
    shaderProgram->addSource("#define COMPUTE_SHADER");

    m_terrainFrustumCullingShader = std::make_shared<Shader>("./Assets/Shader/Terrain/FrustumCulling.comp", false);
    shaderProgram->attachShader(m_terrainFrustumCullingShader);

    shaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    shaderProgram->addSource("#define CBT_READ_ONLY");

    m_terrainCBTShader = std::make_shared<Shader>("./Assets/Shader/Terrain/cbt.comp", false);
    shaderProgram->attachShader(m_terrainCBTShader);
    m_terrainLEBShader= std::make_shared<Shader>("./Assets/Shader/Terrain/leb.comp", false);
    shaderProgram->attachShader(m_terrainLEBShader);
    std::shared_ptr<Shader> atmosphereShader = std::make_shared<Shader>("./Assets/Shader/Terrain/Atmosphere.comp", false);
    shaderProgram->attachShader(atmosphereShader);
    m_terrainRenderCommonShader= std::make_shared<Shader>("./Assets/Shader/Terrain/RenderCommon.comp", false);
    shaderProgram->attachShader(m_terrainRenderCommonShader);
    if (typeFlag == "FLAG_DRAW") {
        std::shared_ptr<Shader> renderDrawShader = std::make_shared<Shader>("./Assets/Shader/Terrain/RenderPass.comp", false);
        shaderProgram->linkCombinedVertexFragment(renderDrawShader, renderDrawShader);
    }   
    else { 
        std::shared_ptr<Shader> renderUpdateShader = std::make_shared<Shader>("./Assets/Shader/Terrain/RenderUpdate.comp", false);
        shaderProgram->addSource("#define " + typeFlag + " 1");
        shaderProgram->attachShader(renderUpdateShader);
        shaderProgram->link();
    }


    HANDLE_GL_ERRORS("setting up " + typeFlag + " shader");
    configureShaderProgram(shaderProgram);
    HANDLE_GL_ERRORS("configuring " + typeFlag + " shader");
}

void Terrain::loadTerrainPrograms()
{
    loadShaderProgram(m_terrainMergeShaderProgram, "FLAG_MERGE");
    loadShaderProgram(m_terrainSplitShaderProgram, "FLAG_SPLIT");
    loadShaderProgram(m_terrainDrawShaderProgram, "FLAG_DRAW");
}

void Terrain::configureTerrainPrograms()
{
    configureShaderProgram(m_terrainMergeShaderProgram);
    configureShaderProgram(m_terrainSplitShaderProgram);
    configureShaderProgram(m_terrainDrawShaderProgram);
}

void Terrain::loadLEBReductionProgram()
{
    m_lebReductionShaderProgram = std::make_shared<ShaderProgram>("LEBReduction");
    std::shared_ptr<Shader> m_cbtCumSumReductionShader = std::make_shared<Shader>("./Assets/Shader/Terrain/cbtSumReduction.comp", false);
    
    m_lebReductionShaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    m_lebReductionShaderProgram->attachShader(m_terrainCBTShader);  // CBT shader already loaded in loadShaderProgram
    m_lebReductionShaderProgram->attachShader(m_cbtCumSumReductionShader);
    m_lebReductionShaderProgram->addSource("#ifdef COMPUTE_SHADER\n#endif");

    m_lebReductionShaderProgram->link();
    HANDLE_GL_ERRORS("loading leb reduction shader");
}

void Terrain::LoadLebReductionPrepassProgram()
{
    m_lebReductionPrepassShaderProgram = std::make_shared<ShaderProgram>("LEBReductionPrepass");
    std::shared_ptr<Shader> m_cbtCumSumReductionPrepassShader = std::make_shared<Shader>("./Assets/Shader/Terrain/cbtSumReductionPrepass.comp", false);
    
    m_lebReductionPrepassShaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    m_lebReductionPrepassShaderProgram->attachShader(m_terrainCBTShader);  // CBT shader already loaded in loadShaderProgram
    m_lebReductionPrepassShaderProgram->attachShader(m_cbtCumSumReductionPrepassShader);
    m_lebReductionPrepassShaderProgram->addSource("#ifdef COMPUTE_SHADER\n#endif");

    m_lebReductionPrepassShaderProgram->link();
    HANDLE_GL_ERRORS("loading leb reduction prepass shader");
}

void Terrain::loadBatchProgram()
{
    m_batchShaderProgram = std::make_shared<ShaderProgram>("Batch");
    std::shared_ptr<Shader> m_batchShader = std::make_shared<Shader>("./Assets/Shader/Terrain/TerrainBatcher.comp", false);
    
    // Enable atomic operations for parallel buffer processing
    m_batchShaderProgram->addSource("#extension GL_ARB_shader_atomic_counter_ops : require");
    m_batchShaderProgram->addSource("#define ATOMIC_COUNTER_EXCHANGE_ARB 1");

    m_batchShaderProgram->addSource("#define FLAG_CS 1");
    m_batchShaderProgram->addSource("#define BUFFER_BINDING_DRAW_ELEMENTS_INDIRECT_COMMAND " + std::to_string(m_bufferTerrainDrawComputeShaderIndex));
    m_batchShaderProgram->addSource("#define BUFFER_BINDING_DISPATCH_INDIRECT_COMMAND " + std::to_string(m_bufferTerrainDispatchComputeShaderIndex));
    m_batchShaderProgram->addSource("#define MESHLET_INDEX_COUNT " + std::to_string(3 << (2 * m_patchSubDiv)));

    m_batchShaderProgram->addSource("#define LEB_BUFFER_COUNT 1");
    m_batchShaderProgram->addSource("#define BUFFER_BINDING_LEB " + std::to_string(m_subdivionBufferIndex));
    m_batchShaderProgram->addSource("#define BUFFER_BINDING_DRAW_ARRAYS_INDIRECT_COMMAND " + std::to_string(m_bufferTerrainDrawIndex));

    m_batchShaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    m_batchShaderProgram->addSource("#define CBT_READ_ONLY");
    
    m_batchShaderProgram->addSource("#define COMPUTE_SHADER");
    m_batchShaderProgram->attachShader(m_terrainCBTShader);  // CBT shader already loaded in loadShaderProgram

    m_batchShaderProgram->attachShader(m_batchShader);
    m_batchShaderProgram->link();
    HANDLE_GL_ERRORS("loading terrain batch shader");
}

void Terrain::loadTopViewProgram()
{
    m_topViewShaderProgram = std::make_shared<ShaderProgram>("TopView");
    std::shared_ptr<Shader> m_topViewShader = std::make_shared<Shader>("./Assets/Shader/Terrain/TopView.comp", false);
    
    m_topViewShaderProgram->addSource("#define FLAG_DISPLACE 1");
    m_topViewShaderProgram->addSource("#define TERRAIN_PATCH_SUBD_LEVEL " + std::to_string(m_patchSubDiv));
    m_topViewShaderProgram->addSource("#define TERRAIN_PATCH_TESS_FACTOR " + std::to_string(1 << m_patchSubDiv));
    m_topViewShaderProgram->addSource("#define BUFFER_BINDING_TERRAIN_VARIABLES " + std::to_string(m_streamTerrainVariablesIndex));
    m_topViewShaderProgram->addSource("#define LEB_BUFFER_COUNT 1");
    m_topViewShaderProgram->addSource("#define BUFFER_BINDING_LEB " + std::to_string(m_subdivionBufferIndex));
    m_topViewShaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    m_topViewShaderProgram->addSource("#define CBT_READ_ONLY");

    m_topViewShaderProgram->attachShader(m_terrainFrustumCullingShader);
    m_topViewShaderProgram->attachShader(m_terrainCBTShader);
    m_topViewShaderProgram->attachShader(m_terrainLEBShader);
    m_topViewShaderProgram->attachShader(m_terrainRenderCommonShader);
    m_topViewShader = std::make_shared<Shader>("./Assets/Shader/Terrain/TopView.comp", false);
    // m_topViewShaderProgram->attachShader(m_topViewShader);
    m_topViewShaderProgram->linkCombinedShader(m_topViewShader, m_topViewShader, m_topViewShader, m_topViewShader, m_topViewShader);
    
    HANDLE_GL_ERRORS("loading terrain top view shader");
    configureTopViewProgram();
}

void Terrain::loadCBTNodeCountShader()
{
    m_cbtNodeCountShaderProgram = std::make_shared<ShaderProgram>("CBTNodeCount");
    std::shared_ptr<Shader> m_cbtNodeCountShader = std::make_shared<Shader>("./Assets/Shader/Terrain/NodeCount.comp", false);
    
    m_cbtNodeCountShaderProgram->addSource("#define CBT_NODE_COUNT_BUFFER_BINDING " + std::to_string(m_bufferCBTNodeCountIndex));
    m_cbtNodeCountShaderProgram->addSource("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(m_subdivionBufferIndex));
    m_cbtNodeCountShaderProgram->addSource("#define CBT_READ_ONLY");
    m_cbtNodeCountShaderProgram->attachShader(m_terrainCBTShader);  // CBT shader already loaded in loadShaderProgram
    m_cbtNodeCountShaderProgram->attachShader(m_cbtNodeCountShader);
    
    m_cbtNodeCountShaderProgram->addSource("#ifdef COMPUTE_SHADER\n#endif");
    m_cbtNodeCountShaderProgram->link();
}

void Terrain::loadTerrainFramebuffer()
{
    LOG_INFO("Loading terrain framebuffer");
    if (glIsFramebuffer(m_framebufferTerrain))
        glDeleteFramebuffers(1, &m_framebufferTerrain);

    glGenFramebuffers(1, &m_framebufferTerrain);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferTerrain);

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
        LOG_ERROR("Error setting up terrain render framebuffer, error code: " << glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Terrain::configureShaderProgram(std::shared_ptr<ShaderProgram> &shaderProgram)
{
    // Calculate LOD
    float tmp = 2.0f * tan(glm::radians(m_scene->getState()->getCamera()->getFov()))
                / m_scene->getState()->getCamera()->getHeight() * (1 << m_patchSubDiv)
                * 7.0f;  // 7.0 is primitivePixelLengthTarget

    float lodFactor = -2.0f * std::log2(tmp) + 2.0f;

    shaderProgram->use();
    shaderProgram->setFloat("u_DmapFactor", m_dmapFactor);
    shaderProgram->setFloat("u_LodFactor", lodFactor);
    shaderProgram->setSampler2D("u_DmapSampler", m_displacementMapTexture, m_displacementMapTexture);
    shaderProgram->setInt("u_SmapSampler", m_slopeMapTexture);
    // shaderProgram->setInt("u_DmapRockSampler", TEXTURE_DMAP_ROCK);
    // shaderProgram->setInt("u_SmapRockSampler", TEXTURE_DMAP_GRASS);
    shaderProgram->setFloat("u_TargetEdgeLength", 7.0f);  // targetEdgeLength
    shaderProgram->setFloat("u_MinLodVariance", glm::sqrt(0.1f / 64.0f / m_dmapFactor));
    shaderProgram->setFloat2("u_ScreenResolution", m_scene->getState()->getCamera()->getWidth(), m_scene->getState()->getCamera()->getHeight());
    shaderProgram->setSampler2D("transmittanceSampler", m_transmittanceTexture, m_transmittanceTexture);
    shaderProgram->setSampler2D("skyIrradianceSampler", m_irradianceTexture, m_irradianceTexture);
    shaderProgram->setSampler3D("inscatterSampler", m_inscatterTexture, m_inscatterTexture);
}

void Terrain::configureAtmosphereProgram()
{
    // atmosphere->setSampler2D("transmittanceSampler", 5, m_transmittanceTexture);
    // shaderProgram->setSampler2D("skyIrradianceSampler", 6, m_irradianceTexture);
    // shaderProgram->setSampler2D("inscatterSampler", 7, m_inscatterTexture);
    // glProgramUniform1i(g_gl.programs[PROGRAM_SKY],
    //                    g_gl.uniforms[UNIFORM_SKY_INSCATTER_SAMPLER],
    //                    TEXTURE_ATMOSPHERE_INSCATTER);
    // glProgramUniform1i(g_gl.programs[PROGRAM_SKY],
    //                    g_gl.uniforms[UNIFORM_SKY_IRRADIANCE_SAMPLER],
    //                    TEXTURE_ATMOSPHERE_IRRADIANCE);
    // glProgramUniform1i(g_gl.programs[PROGRAM_SKY],
    //                    g_gl.uniforms[UNIFORM_SKY_TRANSMITTANCE_SAMPLER],
    //                    TEXTURE_ATMOSPHERE_TRANSMITTANCE);
}

void Terrain::configureTopViewProgram()
{
    m_topViewShaderProgram->use();
    m_topViewShaderProgram->setFloat("u_DmapFactor", m_dmapFactor);
    m_topViewShaderProgram->setSampler2D("u_DmapSampler", 0, m_displacementMapTexture);
    HANDLE_GL_ERRORS("configuring terrain topview shader");
}

bool Terrain::bufferToGL(StreamBuffer *buffer, const void *data, int *offset)
{
    GLint buf = 0;
    void *ptr = NULL;

    // save GL state
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buf);

    // orphaning
    if (buffer->offset + buffer->size > buffer->capacity) {
        buffer->offset = 0;
        // LOG_ERROR("Terrain Stream Buffer orphaned");
    }

    // stream data asynchronously
    glBindBuffer(GL_ARRAY_BUFFER, buffer->gl);
    ptr = glMapBufferRange(
        GL_ARRAY_BUFFER,
        buffer->offset,
        buffer->size,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
    );
    if (!ptr) {
        LOG_ERROR("Error mapping buffer to GL");

        return false;
    }
    memcpy(ptr, data, buffer->size);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, buf);

    // update buffer offset
    if (offset) (*offset) = buffer->offset;
    buffer->offset+= buffer->size;

    return true;
}

GLuint* Terrain::getDrawTextures()
{
    GLuint* drawTextures = new GLuint[3];
    drawTextures[0] = m_framebufferTerrainColorTexture;
    drawTextures[1] = m_framebufferTerrainDepthTexture;
    drawTextures[2] = m_framebufferTerrainTopViewFBO->getColorAttachment(0);
    return drawTextures;
}
