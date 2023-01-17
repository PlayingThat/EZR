//
// Created by maxbr on 22.05.2020.
//

#pragma once

#include "Defs.h"
#include <list>
#include <vector>
#include "Shader.h"

class ShaderProgram
{
public:
    explicit ShaderProgram(std::string name);

    ~ShaderProgram();

    void link(GLenum type = GL_COMPUTE_SHADER);

    void use();

    GLuint getID();

    std::string getName();

    // For normal shaders (compiled already)
    void addShader(std::shared_ptr<Shader> shader);

    // Used for compute shaders whihc include other shaders
    void attachShader(std::shared_ptr<Shader> shader);

    // Used for additional defines in compute shaders
    void addSource(std::string source);

    void setFloat(std::string name, float value) const;

    void setBool(std::string name, bool value) const;

    void setInt(std::string name, int value) const;

    void setVec2(std::string name, glm::vec2 value) const;

    void setVec3(std::string name, glm::vec3 value) const;

    void setVec4(std::string name, glm::vec4 value) const;

    void setMat2(std::string name, glm::mat2 value) const;

    void setMat3(std::string name, glm::mat3 value) const;

    void setMat4(std::string name, glm::mat4 value) const;

    void setSampler2D(std::string name, GLuint texture, int idGl) const;

    void setSampler3D(std::string name, GLuint texture, int idGl) const;

private:
    GLuint compileDirect(const GLchar **sources, int count, GLenum type = GL_COMPUTE_SHADER);
    void writeToFile(std::string source, std::string name);

    GLuint m_id;
    std::list<GLuint> shaders;
    std::vector<std::string> m_sources;
    std::vector<std::shared_ptr<Shader>> m_attachedShaders;

    bool m_linked;
    std::string m_name;
};