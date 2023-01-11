#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gDiffuseColor;

in vec2 uvCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec4 DiffuseColor;

uniform sampler2D diffuseTexture;
uniform sampler2D texture_specular1;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(diffuseTexture, uvCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, uvCoords).r;
    // Diffuse material color without texture information
    gDiffuseColor = DiffuseColor;
}