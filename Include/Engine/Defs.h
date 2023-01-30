//
// Created by maxbr on 10.05.2020.
//

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#define LOG_INFO(info)  std::cout << "Information in " << __FILE__ << "(" << __LINE__ << "): " << info << std::endl << std::flush
#define LOG_WARNING(warning) std::cerr << "Warning in " << __FILE__ << "(" << __LINE__ << "): " << warning << std::endl << std::flush
#define LOG_ERROR(error)  std::cerr << "Error in " << __FILE__ << "(" << __LINE__ << "): " << error << std::endl << std::flush
#define LOG_SHADER_ERROR(shader, error)  std::cerr << "Error in " << shader << ": " << error << std::endl << std::flush

// Pseudo-Macro for clearing all errors in the error buffer
static inline void CLEAR_GL_ERRORS() {
    while (glGetError() != GL_NO_ERROR);
}

// Pseudo-Macro for handling errors at the end of opengl operations
// place_of_occurrence should allow to trace back where the error occured
static inline void HANDLE_GL_ERRORS(std::string place_of_occurrence) {
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR("Error while " << place_of_occurrence << ": " << err);
    }
}
