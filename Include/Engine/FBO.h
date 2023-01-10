//
// Created by maxbr on 24.05.2020.
//

#pragma once

#include <cstdlib>
#include "Defs.h"
#include "State.h"

class Scene;

GLuint createFBO();

void bindFBO(GLuint fboID, size_t width, size_t height);

void unbindFBO(size_t width, size_t height);

GLuint *createColorAttachments(size_t width, size_t height, GLuint numberOfColorAttachments);

GLuint createDepthTextureAttachment(size_t width, size_t height);

GLuint createTextureAttachment(size_t width, size_t height);

GLuint createDepthBufferAttachment(size_t width, size_t height);

class FBO : public SizeCallbackSubscriber
{
public:
    FBO(std::shared_ptr<Scene> scene);

    FBO(std::shared_ptr<Scene> scene, size_t numberOfColorAttachments);

    ~FBO();

    void bind();

    GLuint getID();

    GLuint getColorAttachment(size_t i);

    GLuint getDepthAttachment();

private:
    void setupBuffers(int numberOfColorAttachments);

    void onSizeChanged(int width, int height);

    size_t m_width, m_height;
    GLuint m_fboID;

    size_t m_numberOfColorAttachments;
    GLuint *m_colorAttachments;
    GLuint m_depthAttachment;

    // Scene
    std::shared_ptr<Scene> m_scene;

};
