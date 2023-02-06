//
// Created by jesbu on 17.01.2023.
//

#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;

uniform bool textured;      // If true, the shader will be applied on the textured object

float glowPower = 3.0;              // parameter for adjusting the glow intensity
vec3 glowColor = vec3(0.5,1,1);     //parameter for adjusting the color of the glow

uniform vec3 lightPosition = vec3(0, 10, 4);            //(test) light position in world coordinates

layout (location = 0) out vec4 result;

void main()
{
    // Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec4 vertex = vPosition;
    vec3 ecPos = vertex.xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    //vec3 lightPosition = vec3(gl_LightSource[0].position);

    vec3 lightVec = normalize(lightPosition - ecPos);
    vec3 normalVec = normalize(tNorm);
    vec3 viewVec = normalize(-ecPos);

    float NdotL = clamp(dot(lightVec, normalVec), 0.0, 1.0);

    //glow, based on fresnel
    float glowIntensity = max(0.0, (1.0 - dot(viewVec, normalVec)));    // glow, if view & normal vector point in the same direction
    glowIntensity = pow(glowIntensity, glowPower);                      // control the strength of the glow
    glowIntensity = smoothstep(0.3, 0.4, glowIntensity);                // smooth the glow

    vec4 texColor;
    vec3 diffuseColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize).rgb;

    if (textured) {
        //get the color from the texture
        texColor = texture(textureDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

    //vec3 color = NdotL * diffuseColor + glow * glowColor;
    vec3 color = NdotL * diffuseColor + glowIntensity * glowColor;

    result = vec4(color, 1.0);
}
