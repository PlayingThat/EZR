//
// Created by maxbr on 23.05.2020.
//

#pragma once

#include "Defs.h"
#include <string>
#include <vector>
#include <stb_image.h>
#include <omp.h>

// Used for parallel image loading
struct TextureData
{
    unsigned char* data;
    int width;
    int height;
    int nrComponents;
};
typedef struct TextureData TextureData;

GLuint *loadTexturesInParallel(std::vector<std::string> paths, bool log = true);

GLuint createTextureFromFile(const char* path);

const uint16_t *createTextureFromFile16(const char* path, int &width, int &height);

GLuint createTexture2D(std::size_t width, std::size_t height);
GLuint createGroundTexture2D(std::size_t width, std::size_t height);

GLuint createTexture3D(std::size_t width, std::size_t height, std::size_t depth);
GLuint createWeatherTexture3D(std::size_t width, std::size_t height, std::size_t depth);

GLuint createTexture3D(std::size_t width, std::size_t height, std::size_t depth, float *data);

GLuint createTextureFromFile(std::string path);
GLuint createGroundTextureFromFile(std::string path);

void bindTexture2D(GLuint textureID, unsigned int textureUnit);
void bindGroundTexture2D(GLuint textureID, unsigned int textureUnit);

void bindTexture3D(GLuint textureID, unsigned int textureUnit);