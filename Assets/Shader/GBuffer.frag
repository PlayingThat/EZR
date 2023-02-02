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

in vec3 FragPos;
in vec2 uvCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 WorldPos;
in vec3 Bitangent;

uniform vec4 DiffuseColor;

uniform sampler2D diffuseSampler;
uniform sampler2D metalSampler;
uniform sampler2D smoothnessSampler;
uniform sampler2D heightSampler;
uniform sampler2D normalSampler;
uniform sampler2D ambientOcclusionSampler;

uniform bool isTransparent = false;
uniform float alpha = 1.0;
uniform mat4 projectionMatrix;

// Additional uniforms necessary for parallax mapping
uniform bool UseParallaxMapping;
uniform float HeightScale;

in vec3 cameraPosTangent;
in vec3 lightPosTangent;
in vec3 fragPosTangent;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height = 1.0 - texture(heightSampler, texCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * HeightScale);
    return texCoords - p;
}

// Adapted from https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
vec2 MultiParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * HeightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(heightSampler, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightSampler, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightSampler, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

/// color Regular RGB reflective color of fragment, not pre-multiplied
/// alpha Alpha value of fragment
/// wsZ Window-space-z value == gl_FragCoord.z
vec4 writePixel(vec3 color, float alpha, float wsZ) {
    float ndcZ = 2.0 * wsZ - 1.0;
    // linearize depth for proper depth weighting
    float linearZ = (projectionMatrix[2][2] + 1.0) * wsZ / (projectionMatrix[2][2] + ndcZ);
    float tmp = (1.0 - linearZ) * alpha;
    //float tmp = (1.0 - wsZ * 0.99) * alpha * 10.0; // <-- original weighting function from paper #2
    float w = clamp(tmp * tmp * tmp * tmp * tmp * tmp, 0.0001, 1000.0);
    gUVs.z = alpha * w;
    return vec4(color * alpha* w, alpha);
}

void main()
{    
    vec2 texCoord = uvCoords;

    // Parallel mapping
    if (UseParallaxMapping) {
        // the transpose of texture-to-eye space matrix
        mat3 tbnMatrix;
        tbnMatrix[0] = Tangent;
	    tbnMatrix[1] = Bitangent;
	    tbnMatrix[2] = Normal;
        vec3 viewDir = normalize(cameraPosTangent - fragPosTangent); // tangent space view dir
        texCoord = MultiParallaxMapping(texCoord, viewDir);
    }

    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormals = texture(normalSampler, texCoord);
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    if (isTransparent) {
        gAlbedoSpec = texture(diffuseSampler, uvCoords);
        gDiffuseColor = DiffuseColor;
    }
    else {
        gAlbedoSpec.rgb = texture(diffuseSampler, uvCoords).rgb;
        gDiffuseColor = DiffuseColor;
    }
        
    // Diffuse material color without texture information
    // store the UV coordinates in the 4th gbuffer texture
    gUVs = vec4(texCoord, 0.0, 0.0);
    // store the tangent space basis vectors in the 5th gbuffer texture
    gTangents = vec4(Tangent, 0.0);

    // Special PBR textures
    gMetalSmoothnessAOHeight.r = texture(metalSampler, texCoord).r;
    gMetalSmoothnessAOHeight.g = texture(smoothnessSampler, texCoord).r;
    gMetalSmoothnessAOHeight.b = texture(heightSampler, texCoord).r;
    gMetalSmoothnessAOHeight.a = texture(ambientOcclusionSampler, texCoord).r;
}