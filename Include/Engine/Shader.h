//
// Created by maxbr on 22.05.2020.
//

#pragma once

#include "Defs.h"
#include <fstream>
#include <sstream>
#include <utility>

struct shaderType
{
    shaderType() : glType(0), name("")
    {}

    shaderType(unsigned int glType, std::string name) : glType(glType), name(std::move(name))
    {}

    unsigned int glType;
    std::string name;
};

class Shader
{
public:
    explicit Shader(std::string path, bool compile = true);

    ~Shader();

    void compileShader();

    const GLuint getID() const;

    std::string getSource();

    const shaderType &getType() const;

private:
    std::string loadFromFile(std::string path);

    shaderType getTypeFromPath(std::string path);

    GLuint m_id;
    std::ifstream m_shaderFileStream;
    bool m_compiled;
    shaderType m_type;
    std::string m_path;
};