#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangents;
layout (location = 4) in vec3 aBitangents;

out vec3 FragPos;
out vec2 uvCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 WorldPos;
out vec3 Bitangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

// Parallax mapping
uniform vec3 cameraPosition;
uniform vec3 lightPosition;

out vec3 cameraPosTangent;
out vec3 lightPosTangent;
out vec3 fragPosTangent;

void main()
{
    vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    uvCoords = aTexCoords;
    
    Normal = normalMatrix * aNormal;

    Tangent = (modelMatrix * vec4(aTangents, 0.0)).xyz;

    WorldPos = worldPos.xyz;

    Bitangent = aBitangents;
    vec3 T = normalize(mat3(modelMatrix) * aTangents);
    vec3 B = normalize(mat3(modelMatrix) * aBitangents);
    vec3 N = normalize(mat3(modelMatrix) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));
    cameraPosTangent = TBN * cameraPosition;
    lightPosTangent = TBN * lightPosition;
    fragPosTangent = TBN * FragPos;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}