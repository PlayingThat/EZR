// shader for imitating an oil painting

// Based on Anisotropic Kuwahara Filtering on the GPU
// by Jan Eric Kyprianidis, Henry Kang, and Jürgen Döllner

//
// Created by jesbu on 07.02.2023.
//


#version 450

layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}