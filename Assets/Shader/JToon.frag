//
// Created by jesbu on 11.01.2023.
//

#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;

uniform bool Textured;      // If true, the shader will be applied on the textured object

uniform int colorLevels;                        // adjustable number of different color levels
const float scaleFactor = 1.0f / colorLevels;   // width of a color level

uniform float levelBrightness;                  // adjustable parameter to brighten the result 

vec3 lightPosition = vec3(0, 10, 4);            //(test) light position in world coordinates

layout (location = 0) out vec4 result;

void main()
{
    // Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec4 vertex = vPosition;
    vec3 ecPos = vertex.xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    //vec3 lightPosition = vec3(gl_LightSource[0].position);

    vec3 lightVec = normalize(lightPosition - ecPos);
    vec3 normalVec = normalize(tNorm);
    
    //calculate the intensity using the lightvector & normalvector
    float intensity = max(dot(lightVec, normalVec), 0.0); 

    //variable for storing the color level of the intensity
    float intensityLevel = 0.0;

    /*
    //test: set the intensity for 3 color levels
    if (intensity > 0.75) {
 		intensity = 1;
 	}
 	else if (intensity > 0.5) {
 		intensity = 0.75;
 	}
    else {
 		intensity = 0.5;
 	}
    */

    // loop for x color levels (I am so proud of this loop)
    for (int i = 0; i < colorLevels; i++){
        if (intensity >= i * scaleFactor){
            intensityLevel = (i* scaleFactor);    
        }
    }

    vec4 texColor;
    vec3 diffuseColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize).rgb;

    if (Textured) {
        //get the color from the texture
        texColor = texture(textureDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

    // get the color by multiplying the diffuse color with the intensity level
    vec3 color = diffuseColor * (intensityLevel + levelBrightness);     //add Offset, so the result is brighter

    // alternative solution (dark shadows from the dark internet, for comparison)
    vec3 color2 = diffuseColor * floor(intensity * colorLevels)* scaleFactor;

    result = vec4(color, 1.0);
}
