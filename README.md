
# Overview
![Dynamic Terrain Division](https://github.com/PlayingThat/EZR/blob/main/EZR.gif)
![Non-photorealistic rendering effects](https://github.com/PlayingThat/EZR/blob/main/NPR.gif)

-   OpenGL rendering of terrain with dynamic LOD
    -   The system is based on Binary Trees: https://onrendering.com/data/papers/cbt/ConcurrentBinaryTrees.pdf
    -   Parallelized division and splitting on the GPU
-   Various non-photorealistic rendering effects
-   Volumetric clouds

# Setup

1.  Under Windows, run  `build_dependencies.bat`  as administrator. This should build glfw and glm and set the environment variables automatically.
2.  Open the EZR project in VS Code/CLion/whatever and check if it works. OpenGL should be found automatically.

## Dependencies

-   OpenGL 4.5

### Included

-   ImGui: https://github.com/ocornut/imgui
-   Assimp: https://github.com/assimp/assimp

# Info

-   Use  `Engine`  for the basic functionality of the framework, but  `Objects`  should manage their own special features (see  `Clouds`  as an example).
-   Use smart pointers, i.e.  `make_unique/make_shared`  instead of  `new`. There are also examples of this in the code.
-   ImGui provides the user interface.
-   There is a shader management system, so that uniforms can be set more easily.