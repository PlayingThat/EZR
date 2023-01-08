#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec3 normal;
out vec3 position;

void main()
{
    mat4 normalMatrix =  transpose ( inverse ( viewMatrix * modelMatrix));
    normal = (normalize( normalMatrix * vec4(aNormal.xyz, 0.0))).xyz;

    position = (viewMatrix * modelMatrix * vec4(aPos.xyz, 1.0)).xyz;

    gl_Position = projectionMatrix * vec4(position, 1.0);
}