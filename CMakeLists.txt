project(EZR)
cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_DIR Include)

# use PACKAGE_ROOT variables
cmake_policy(SET CMP0074 NEW)

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

include_directories(Include)

set(SRC_LIST Include/stb_image.h
        Include/imgui/imconfig.h
        Include/imgui/imgui.cpp
        Include/imgui/imgui.h
        Include/imgui/imgui_demo.cpp
        Include/imgui/imgui_draw.cpp
        Include/imgui/imgui_impl_glfw.cpp
        Include/imgui/imgui_impl_glfw.h
        Include/imgui/imgui_impl_opengl3.cpp
        Include/imgui/imgui_impl_opengl3.h
        Include/imgui/imgui_internal.h
        Include/imgui/imgui_widgets.cpp
        Include/imgui/imgui_impl_opengl3_loader.h
        Include/imgui/imstb_rectpack.h
        Include/imgui/imgui_tables.cpp
        Include/imgui/imstb_textedit.h
        Include/imgui/imstb_truetype.h
        Source/main.cpp
        Source/Objects/Triangle.cpp
        Source/Engine/Drawable.cpp
        Source/Engine/Camera.cpp
        Source/Engine/ShaderProgram.cpp
        Source/Engine/Shader.cpp
        Source/Engine/Texture.cpp
        Source/Engine/FBO.cpp
        Source/Objects/ScreenFillingQuad.cpp
        Source/Objects/Quad.cpp
        Source/Engine/State.cpp
        Source/Engine/Scene.cpp
        Source/Objects/ColorfullTriangle.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} )

target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${GLM_INCLUDE_PATH} Include)

# glad
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

target_link_libraries(
        ${PROJECT_NAME}
        ${OpenGL3_LIBRARIES}
        GLFW3::glfw3
        GLM::glm
        opengl32
        glad
        imm32)

# Copy Assets
add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/Assets
                ${CMAKE_BINARY_DIR}/Assets)

# prevent console from opening
#list(APPEND CMAKE_CXX_FLAGS "-mwindows")

# Disable testing
set (ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DBUILD_TESTING:BOOL=OFF")