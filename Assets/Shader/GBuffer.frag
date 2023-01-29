#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gUVs;
layout (location = 3) out vec4 gTangents;
layout (location = 4) out vec4 gAlbedoSpec;
layout (location = 5) out vec4 gDiffuseColor;

// Special textures
layout (location = 6) out vec4 gMetalSmoothnessAOHeight;
layout (location = 7) out vec4 gNormals;

in vec2 uvCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;

uniform vec4 DiffuseColor;

uniform sampler2D diffuseSampler;
uniform sampler2D metalSampler;
uniform sampler2D smoothnessSampler;
uniform sampler2D heightSampler;
uniform sampler2D normalSampler;
uniform sampler2D ambientOcclusionSampler;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(diffuseSampler, uvCoords).rgb;
    // Diffuse material color without texture information
    gDiffuseColor = DiffuseColor;
    // store the UV coordinates in the 4th gbuffer texture
    gUVs = vec4(uvCoords, 0.0, 0.0);
    // store the tangent space basis vectors in the 5th gbuffer texture
    gTangents = vec4(Tangent, 0.0);

    // Special PBR textures
    gMetalSmoothnessAOHeight.r = texture(metalSampler, uvCoords).r;
    gMetalSmoothnessAOHeight.g = texture(smoothnessSampler, uvCoords).r;
    gMetalSmoothnessAOHeight.b = texture(heightSampler, uvCoords).r;
    gMetalSmoothnessAOHeight.a = texture(ambientOcclusionSampler, uvCoords).r;
    gNormals = texture(normalSampler, uvCoords);
}