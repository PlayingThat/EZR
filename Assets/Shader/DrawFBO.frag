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

// Transparancy
uniform bool transparency = false;
uniform sampler2D transparencySampler;
uniform sampler2D transparencyTextureSampler;
uniform sampler2D transparencyDiffuseSampler;
uniform sampler2D transparencyDepth;

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
    float depthFrag = texture(depth, gl_FragCoord.xy / screenSize).r;

    // Depth test for terrain vs scene objects
    float terrainDepth = texture(terrainDepth, gl_FragCoord.xy / screenSize).r;
    bool terrainVisible = terrainDepth < 1 && depthFrag > terrainDepth;
    if (terrainVisible) {
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
    if (depthFrag == 1.0f && !terrainVisible) {
        clouds = texture(fboClouds, gl_FragCoord.xy / screenSize).rgb;
        FragColor = vec4(clouds, 1.0f);
    }
    else
        FragColor = color;

    // Blend transparency
    if(transparency) {
        float transparentDepth = texture(transparencyDepth, gl_FragCoord.xy / screenSize).r;
        if (transparentDepth < depthFrag) {
            float alpha = 0.5f;
            vec4 transparencyTexture = texture(transparencyTextureSampler, gl_FragCoord.xy / screenSize);
            // vec4 transparencyDiffuse = texture(transparencyDiffuseSampler, gl_FragCoord.xy / screenSize);
            FragColor = FragColor * (1.0f - alpha) + transparencyTexture * alpha;
        }
    }
}