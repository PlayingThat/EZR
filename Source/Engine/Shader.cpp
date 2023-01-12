//
// Created by maxbr on 22.05.2020.
//

#include "Engine/Shader.h"

Shader::Shader(std::string path, bool compile)
{
    m_compiled = compile;

    m_type = getTypeFromPath(path);
    m_path = path;

    // compile shader
    if (compile)
    {
        compileShader();
    }
}

void Shader::compileShader()
{
    // try to load the shader from file
    std::string shaderString = loadFromFile(m_path);
    const char *shaderCString = shaderString.c_str();

    m_id = glCreateShader(m_type.glType);
    glShaderSource(m_id, 1, &shaderCString, NULL);
    glCompileShader(m_id);

    int success;
    // if an error occured, write it to log
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(m_id, 1024, NULL, infoLog);
        LOG_SHADER_ERROR(m_type.name << " shader", "Shader compilation failed on " + m_type.name + " '" + m_path + "': " + infoLog);
    }
}

std::string Shader::getSource()
{
    return loadFromFile(m_path).c_str();
}

Shader::~Shader()
{
    glDeleteShader(m_id);
}

const GLuint Shader::getID() const
{
    return m_id;
}

std::string Shader::loadFromFile(std::string path)
{
    // allow exceptions to be thrown
    m_shaderFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        m_shaderFileStream.open(path);
        std::stringstream stringStream;

        stringStream << m_shaderFileStream.rdbuf();

        m_shaderFileStream.close();
        return stringStream.str();
    } catch (std::ifstream::failure &f)
    {
        LOG_ERROR("Couldn't load " + m_type.name + " shader : '" + path + "' due to " + f.what());
    }

    return std::string();
}

shaderType Shader::getTypeFromPath(std::string path)
{
    // get file extension from path
    std::string fileName = path.substr(path.size() - 4);

    if(fileName == "vert")
        return shaderType(GL_VERTEX_SHADER, "Vertex");
    if(fileName == "frag")
        return shaderType(GL_FRAGMENT_SHADER, "Fragment");
    if(fileName == "comp")
        return shaderType(GL_COMPUTE_SHADER, "Compute");

    LOG_ERROR("Couldn't match shader type on shader '" + path + "'");
    return shaderType(0, std::string());
}

const shaderType &Shader::getType() const
{
    return m_type;
}
