//
// Created by maxbr on 23.05.2020.
//

#include "Engine/Texture.h"

GLuint createTextureFromFile(const char* path)
{
    // Load image from file
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

    // OpenGL texture handle
    GLuint texHandle;
    glGenTextures(1, &texHandle);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Free image memory
        stbi_image_free(data);
    }
    else
    {
        LOG_ERROR("Texture failed to load at path: " << path);
        // Free image memory
        stbi_image_free(data);
    }

    return texHandle;
}

const uint16_t *createTextureFromFile16(const char* path, int &width, int &height) 
{
    const uint16_t * img_data = stbi_load_16(path, (int*)&width, (int*)&height, nullptr, 1);
    if(img_data == nullptr) {
        LOG_ERROR(stbi_failure_reason());
    }
    return img_data;
}


GLuint createTexture2D(std::size_t width, std::size_t height)
{
    GLuint texHandle;
    glGenTextures(1, &texHandle);

    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    return texHandle;
}

GLuint createGroundTexture2D(std::size_t width, std::size_t height)
{
    GLuint texHandle;
    glGenTextures(1, &texHandle);

    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    return texHandle;
}

GLuint createTexture3D(std::size_t width, std::size_t height, std::size_t depth)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_3D, textureId);

    // set some usable default tex parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, nullptr);

    return textureId;
}

GLuint createWeatherTexture3D(std::size_t width, std::size_t height, std::size_t depth)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_3D, textureId);

    // set some usable default tex parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, nullptr);

    // generate mipmap
    glGenerateTextureMipmap(GL_TEXTURE_3D);

    // bind level of texture to texture unit
    glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return textureId;
}

GLuint createTexture3D(std::size_t width, std::size_t height, std::size_t depth, float *data)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_3D, textureId);

    // set some usable default tex parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, (GLvoid *) data);

    // generate mipmap
    glGenerateTextureMipmap(GL_TEXTURE_3D);

    return textureId;
}

GLuint createTextureFromFile(std::string path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if(data)
    {
        // account for possible alpha channel in the data
        assert((1 <= nrChannels) && (4 >= nrChannels));
        GLenum glformat;
        switch(nrChannels)
        {
            case 1:
                glformat = GL_RED;
                break;
            case 2:
                glformat = GL_RG8;
                break;
            case 3:
                glformat = GL_RGB8;
                break;
            case 4:
                glformat = GL_RGBA8;
                break;
        }

        if (glformat == GL_RGBA8)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else
    {
        LOG_ERROR("failed to load texture " + path);
    }
    stbi_image_free(data);

    return texture;
}

GLuint createGroundTextureFromFile(std::string path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if(data)
    {
        // account for possible alpha channel in the data
        assert((1 <= nrChannels) && (4 >= nrChannels));
        GLenum glformat;
        switch(nrChannels)
        {
            case 1:
                glformat = GL_RED;
                break;
            case 2:
                glformat = GL_RG8;
                break;
            case 3:
                glformat = GL_RGB8;
                break;
            case 4:
                glformat = GL_RGBA8;
                break;
        }

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, 3, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_3D);

    } else
    {
        LOG_ERROR("failed to load texture" + path);
    }
    stbi_image_free(data);

    return texture;
}

void bindTexture2D(GLuint textureID, unsigned int textureUnit)
{
    glBindImageTexture(textureUnit, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
}

void bindTexture3D(GLuint textureID, unsigned int textureUnit)
{
    glBindImageTexture(textureUnit, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}