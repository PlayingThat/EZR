cd Dependencies/glfw
cmake . -DGLFW_BUILD_EXAMPLES=OFF -G "MinGW Makefiles"
cmake --build .
xcopy %cd%\src\libglfw3.a %cd%\lib\libglfw3.a*
setx GLFW3_ROOT "%cd%"
cd ../glm
cmake . -G "MinGW Makefiles"
cmake --build .
setx GLM_ROOT "%cd%"
