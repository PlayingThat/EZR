//
// Created by maxbr on 24.05.2020.
//

#include "Engine/FBO.h"

#include "Engine/Scene.h"

void bindFBO(GLuint fboID, size_t width, size_t height)
{
    // unbind textures that may already be bound
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    glViewport(0, 0, width, height);
}

void unbindFBO(size_t width, size_t height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

GLuint createFBO()
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    return fbo;
}

GLuint *createColorAttachments(size_t width, size_t height, GLuint numberOfColorAttachments)
{
    GLuint *colorAttachments = new GLuint[numberOfColorAttachments];

    unsigned int *attachments = new unsigned int[numberOfColorAttachments];

    glGenTextures(numberOfColorAttachments, colorAttachments);

    for(GLuint i = 0; i < numberOfColorAttachments; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments[i], 0);

        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    
    glDrawBuffers(numberOfColorAttachments, attachments);
    return colorAttachments;
}

GLuint createTextureAttachment(size_t width, size_t height)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    return textureID;
}

GLuint createDepthBufferAttachment(size_t width, size_t height)
{
    GLuint depthAttachement;
    glGenTextures(1, &depthAttachement);


    glBindTexture(GL_TEXTURE_2D, depthAttachement);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    return depthAttachement;
}

FBO::FBO(std::shared_ptr<Scene> scene)
{
    m_scene = scene;
    // Subscribe to size change events
    m_scene->getState()->attachWindowSizeChangeCallback(this);

    setupBuffers(0);
}

FBO::FBO(std::shared_ptr<Scene> scene, size_t numberOfColorAttachments) 
{
    m_scene = scene;
    // Subscribe to size change events
    m_scene->getState()->attachWindowSizeChangeCallback(this);

    setupBuffers(numberOfColorAttachments);
}

void FBO::setupBuffers(int numberOfColorAttachments)
{
    m_width = m_scene->getState()->getCamera()->getWidth();
    m_height = m_scene->getState()->getCamera()->getHeight();

    m_fboID = createFBO();
    m_numberOfColorAttachments = numberOfColorAttachments;
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    m_colorAttachments = createColorAttachments(m_width, m_height, m_numberOfColorAttachments);

    // Create depth buffer
    // unsigned int m_depthAttachment;
    // glGenRenderbuffers(1, &m_depthAttachment);
    // glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);

    glGenTextures(1, &m_depthAttachment);
    glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);  


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer not complete");
}

FBO::~FBO()
{

}

// Callback for size change will need to recreate the buffers
void FBO::onSizeChanged(int width, int height)
{
    m_width = width;
    m_height = height;
    setupBuffers(m_numberOfColorAttachments);
    LOG_INFO("Size changed to: " + std::to_string(width) + "x" + std::to_string(height));
}

void FBO::bind()
{
    bindFBO(m_fboID, m_width, m_height);
}

GLuint FBO::getID()
{
    return m_fboID;
}

GLuint FBO::getColorAttachment(size_t i)
{
    if(i > m_numberOfColorAttachments)
    {
        LOG_ERROR("Color attachment index out of range!");
        return 0;
    }
    return m_colorAttachments[i];
}

GLuint FBO::getDepthAttachment()
{
    return m_depthAttachment;
}

size_t FBO::getWidth()
{
    return m_width;
}

size_t FBO::getHeight()
{
    return m_height;
}

void FBO::copyToFBO(std::shared_ptr<FBO> fbo)
{
    // Copy color attachments
    for (size_t i = 0; i < m_numberOfColorAttachments; i++)
    {
        glCopyImageSubData(m_colorAttachments[i], GL_TEXTURE_2D, 0, 0, 0, 0, fbo->getColorAttachment(i), GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    }
    // Copy depth attachment
    glCopyImageSubData(m_depthAttachment, GL_TEXTURE_2D, 0, 0, 0, 0, fbo->getDepthAttachment(), GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    HANDLE_GL_ERRORS("copying FBO");
}
