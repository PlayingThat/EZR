//
// Created by maxb on 27.01.2023.
//

#version 450 core

out vec4 FragColor;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D uvs;
uniform sampler2D depth;
uniform sampler2D tangents;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;

// Load parameters from gbuffer
vec2 TexCoords = texture(uvs, gl_FragCoord.xy / screenSize).xy;
vec3 WorldPos = texture(positions, gl_FragCoord.xy / screenSize).xyz;
vec3 Normal = texture(normals, gl_FragCoord.xy / screenSize).xyz;

// material parameters
uniform sampler2D textureNormal;
uniform sampler2D textureMetalSmoothnessAOHeight;

// lights
uniform vec3 lightPosition = vec3(0, 10, 4);
uniform vec4 lightColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);

uniform vec3 cameraPosition;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(textureNormal, gl_FragCoord.xy / screenSize).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(gl_FragCoord.xy / screenSize);
    vec2 st2 = dFdy(gl_FragCoord.xy / screenSize);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    // retrieve material information from gbuffer
    vec3 albedo     = pow(texture(textureDiffuse, gl_FragCoord.xy / screenSize).rgb, vec3(2.2));
    vec4 metalSmoothnessAOHeight = texture(textureMetalSmoothnessAOHeight, gl_FragCoord.xy / screenSize);
    float metallic  = metalSmoothnessAOHeight.r;
    float roughness = metalSmoothnessAOHeight.g;
    float ao        = metalSmoothnessAOHeight.b;
    float height    = metalSmoothnessAOHeight.a;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(cameraPosition - WorldPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = normalize(lightPosition - WorldPos);
    vec3 H = normalize(V + L);

    vec3 radiance = lightColor.rgb;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
    vec3 nominator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.01f; // 0.001 to prevent divide by zero.
    vec3 brdf = nominator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again  
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.05) * albedo * ao * lightColor.a;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/3.2));

    FragColor = vec4(color, 1.0);
}