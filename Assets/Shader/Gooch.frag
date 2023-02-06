//
// Created by jesbu on 03.01.2023.
//

#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;

uniform bool Textured;      // If true, the shader will be applied on the textured object

uniform float DiffuseCool;
uniform float DiffuseWarm;
//vec3 CoolColor = vec3(0.35, 0.45, 0.95);    //cold blue color
//vec3 WarmColor = vec3(1, 0.59, 0.4);        //warm orange color
uniform vec3 CoolColor;
uniform vec3 WarmColor;

uniform bool UseSun;
uniform vec3 lightPosition;        
vec3 testLightPosition = vec3(0, 10, 4);    //(test) light position in world coordinates

layout (location = 0) out vec4 result;

void main()
{
    // Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec3 ecPos = vPosition.xyz;
    vec3 viewVec = normalize(-ecPos);

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;
    vec3 normalVec = normalize(tNorm);

    vec3 lightVec;
    if (UseSun){        // use sun light for more realism
        lightVec = normalize(lightPosition - ecPos);
    }
    else {              // use test light for presentation purposes    
        lightVec = normalize(testLightPosition - ecPos);
        
    }

    vec3 reflecVec = normalize(reflect(-lightVec, normalVec));

    float NdotL = (dot(lightVec, normalVec) + 1.0) * 0.5;

    vec4 texColor;
    vec3 diffuseColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize).rgb;
    
    if (Textured) {
        //get the color from the texture
        texColor = texture(textureDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

    //combine cool / warm color with vertex color
    vec3 cool = min(CoolColor + DiffuseCool * diffuseColor, 1.0);
    vec3 warm = min(WarmColor + DiffuseWarm * diffuseColor, 1.0);

    //interpolate using the dot product of normal & lightvector
    vec3 final = mix(cool, warm, NdotL);
 
    //set the specular highlight using the reflected lightvector & viewvector
    vec3 nRefl = normalize(reflecVec);
    vec3 nView = normalize(viewVec);
    float spec = pow(max(dot(nRefl, nView), 0.0), 32.0);
    
    result = vec4(min(final + spec, 1.0), 1.0);
}
