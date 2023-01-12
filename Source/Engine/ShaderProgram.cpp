//
// Created by maxbr on 22.05.2020.
//

#include "Engine/ShaderProgram.h"

ShaderProgram::ShaderProgram(std::string name)
{
    m_linked = false;
    m_name = name;
    m_id = glCreateProgram();

    m_attachedShaders = std::vector<std::shared_ptr<Shader>>();
    m_sources = std::vector<GLchar *>();
    addSource("#version 450");
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_id);
}

GLuint ShaderProgram::compileDirect(const char **sources, int count)
{

    GLuint shaderId = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shaderId, count, sources, NULL);
    glCompileShader(shaderId);

    int success;
    // if an error occured, write it to log
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
        LOG_SHADER_ERROR("Compute shader", "Shader compilation failed on Compute Shader '<Concatenated from sources>': " << infoLog);
    }
    return shaderId;
}

void ShaderProgram::link()
{
    // If no compiled shaders are attached, attach sources and shaders first
    if(m_attachedShaders.size() > 0)
    {
        // determine source and length of source pointer array
        int srcc = m_sources.size() + m_attachedShaders.size();
        char *source = new char[1000000]();

        int j = 0;
        // Add sources (definitions etc.) before shader
        for(int i = 0; i < m_sources.size(); i++)
        {
            strcat(source, m_sources[i]);
            j++;
        }
        
        // Compile shaders
        for(int i = 0; i < m_attachedShaders.size(); i++)
        {
            strcat(source, m_attachedShaders[i]->getSource().c_str());
            j++;
        }

        const char *sourceArray[] = {source};
        // writeToFile(source, "combinedShader.txt");
        // create dummy shader 
        GLuint dummyShader = compileDirect(sourceArray, 1);
        glAttachShader(m_id, dummyShader);
    }

    glLinkProgram(m_id);

    // if errors occured, write them to log
    GLint success = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(m_id, 1024, NULL, infoLog);
        LOG_SHADER_ERROR("Shader program", "Shader program " + m_name + " linking failed: " + infoLog);
        glDeleteProgram(m_id);
    }

    m_linked = true;
}

void ShaderProgram::writeToFile(char* source, std::string name)
{
    std::ofstream file;
    file.open(name);
    file << source;
    file.close();
}

void ShaderProgram::use()
{
    if(m_linked)
        glUseProgram(m_id);
    else
        LOG_WARNING(" Shader program " + m_name + " can't be used before being linked.");
}

GLuint ShaderProgram::getID()
{
    return m_id;
}

std::string ShaderProgram::getName()
{
    return m_name;
}

void ShaderProgram::addShader(std::shared_ptr<Shader> shader)
{
    shaders.push_back(shader->getID());
    glAttachShader(m_id, shader->getID());
}

void ShaderProgram::attachShader(std::shared_ptr<Shader> shader)
{
    m_attachedShaders.push_back(shader);
}

void ShaderProgram::addSource(std::string source)
{
    GLchar *temp = strdup(source.c_str());
    m_sources.push_back(strcat(temp, "\n"));
}

void ShaderProgram::setFloat(std::string name, float value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(uniformLocation, value);
}

void ShaderProgram::setBool(std::string name, bool value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(uniformLocation, value != 0);
}

void ShaderProgram::setInt(std::string name, int value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(uniformLocation, value);
}

void ShaderProgram::setVec2(std::string name, glm::vec2 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform2fv(uniformLocation, 1, glm::value_ptr(value));
}

void ShaderProgram::setVec3(std::string name, glm::vec3 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
}

void ShaderProgram::setVec4(std::string name, glm::vec4 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniform4fv(uniformLocation, 1, glm::value_ptr(value));
}

void ShaderProgram::setMat2(std::string name, glm::mat2 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix2fv(uniformLocation, 1, false, glm::value_ptr(value));
}

void ShaderProgram::setMat3(std::string name, glm::mat3 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix3fv(uniformLocation, 1, false, glm::value_ptr(value));
}

void ShaderProgram::setMat4(std::string name, glm::mat4 value) const
{
    GLuint uniformLocation = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(value));
}

void ShaderProgram::setSampler2D(std::string name, GLuint texture, int idGl) const
{
    glActiveTexture(GL_TEXTURE0 + texture);
    glBindTexture(GL_TEXTURE_2D, idGl);
    this->setInt(name, texture);
}

void ShaderProgram::setSampler3D(std::string name, GLuint texture, int idGl) const
{
    glActiveTexture(GL_TEXTURE0 + idGl);
    glBindTexture(GL_TEXTURE_3D, texture);
    this->setInt(name, idGl);
}
