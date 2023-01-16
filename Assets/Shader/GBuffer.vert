#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangents;

out vec3 FragPos;
out vec2 uvCoords;
out vec3 Normal;
out vec3 Tangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    uvCoords = aTexCoords;
    
    Normal = normalMatrix * aNormal;

    Tangent = (modelMatrix * vec4(aTangents, 0.0)).xyz;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}