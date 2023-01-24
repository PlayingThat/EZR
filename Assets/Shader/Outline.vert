#version 450

layout (location = 0) in vec2 pos;
layout (location = 2) in vec2 aTexCoords;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}