#version 450 core

uniform sampler2D fbo0;
uniform sampler2D fbo1;
uniform sampler2D fbo2;
uniform sampler2D fbo3;
uniform sampler2D fbo4;
uniform sampler2D fbo5;
uniform sampler2D fbo6;
uniform sampler2D fbo7;
uniform sampler2D fboClouds;

// mask for clouds
uniform sampler2D depth;

// Terrain color buffer
uniform sampler2D terrain;
uniform sampler2D terrainDepth;
uniform sampler2D terrainTopView;

uniform vec2 screenSize;
uniform int numberOfEnabledEffects;

out vec4 FragColor;

void main()
{
    // Early exit for terrain top view area
    vec4 terrainTopViewFrag = texture(terrainTopView, gl_FragCoord.xy / screenSize);
    if (terrainTopViewFrag.r > 0.0f) {
        FragColor = terrainTopViewFrag;
        return;
    }

    vec4 color;
    float effectPercentage = 1.0 / numberOfEnabledEffects;

    vec4 terrainFrag = texture(terrain, gl_FragCoord.xy / screenSize);
    vec4 depthFrag = texture(depth, gl_FragCoord.xy / screenSize);

    if (texture(terrainDepth, gl_FragCoord.xy / screenSize).r < depthFrag.r) {
        color = terrainFrag;
    }
    else {
        color = texture(fbo0, gl_FragCoord.xy / screenSize);
    }

    // Blend the enabled effects together
    if (numberOfEnabledEffects > 0)
        color *= effectPercentage;
    if (numberOfEnabledEffects > 1)
        color += texture(fbo1, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 2)
        color += texture(fbo2, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 3)
        color += texture(fbo3, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 4)
        color += texture(fbo4, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 5)
        color += texture(fbo5, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 6)
        color += texture(fbo6, gl_FragCoord.xy / screenSize) * effectPercentage;
    if (numberOfEnabledEffects > 7)
        color += texture(fbo7, gl_FragCoord.xy / screenSize) * effectPercentage;

    vec3 clouds;
    if (depthFrag.r == 1.0f) {
        clouds = texture(fboClouds, gl_FragCoord.xy / screenSize).rgb;
        FragColor = vec4(clouds, 1.0f);
    }
    else
        FragColor = color;
    //vec4 terrainFrag = texture(terrain, gl_FragCoord.xy / screenSize);
    //FragColor = texture(terrain, gl_FragCoord.xy / screenSize);
}