#version 450 core

uniform sampler2D positions;
uniform sampler2D normals;

uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize);
}