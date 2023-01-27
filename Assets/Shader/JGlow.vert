//shader for glowing objects
//based on Rim Lighting & Fresnel
//Rim Lighting: non-realistic rendering
//Fresnel: describes the reflection / transmission of light

//
// Created by jesbu on 17.01.2023.
//


#version 450

layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}