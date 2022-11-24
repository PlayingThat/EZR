cd Dependencies/glfw
cmake . -DGLFW_BUILD_EXAMPLES=OFF -G "MinGW Makefiles"
cmake --build . -j %NUMBER_OF_PROCESSORS%
xcopy %cd%\src\libglfw3.a %cd%\lib\libglfw3.a* /d
setx GLFW3_ROOT "%cd%"
cd ../glm
cmake . -G "MinGW Makefiles"
cmake --build . -j %NUMBER_OF_PROCESSORS%
setx GLM_ROOT "%cd%"
cd ../assimp
cmake . -DBUILD_SHARED_LIBS=ON -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_NO_EXPORT=ON -G "MinGW Makefiles"
cmake --build . -j %NUMBER_OF_PROCESSORS%