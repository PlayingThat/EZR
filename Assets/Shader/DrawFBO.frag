#version 450 core

uniform sampler2D fbo0;
uniform sampler2D fbo1;
uniform sampler2D fbo2;
uniform sampler2D fbo3;
uniform sampler2D fbo4;
uniform sampler2D fbo5;
uniform sampler2D fbo6;
uniform sampler2D fbo7;

uniform vec2 screenSize;
uniform int numberOfEnabledEffects;

out vec4 FragColor;

void main()
{
    vec4 color;
    float effectPercentage = 1.0 / numberOfEnabledEffects;

    // Blend the enabled effects together
    if (numberOfEnabledEffects > 0)
        color = texture(fbo0, gl_FragCoord.xy / screenSize) * effectPercentage;
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

    FragColor = color;
}