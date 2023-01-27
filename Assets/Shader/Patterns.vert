//
// Created by jesbu on 22.01.2023.
//
// a shader for stippling and hatching patterns
// based on http://www.yaldex.com/open-gl/ch18lev1sec1.html

#version 450

layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}