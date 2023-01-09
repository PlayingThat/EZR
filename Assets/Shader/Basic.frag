#version 450 core

uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

out vec4 FragColor;

void main()
{
    FragColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize);
}