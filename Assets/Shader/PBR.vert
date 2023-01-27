//
// Created by maxb on 27.01.2023.
//

#version 450

layout (location = 0) in vec2 pos;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}
