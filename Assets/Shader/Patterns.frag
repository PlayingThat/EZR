//
// Created by jesbu on 22.01.2023.
//
// a shader for stippling and hatching patterns
// based on http://www.yaldex.com/open-gl/ch18lev1sec1.html


#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;

uniform bool Colored;       // If true, the shader will use the base colors of the object
uniform bool Textured;      // If true, the shader will be applied on the textured object

uniform int mode;           // Dots, Vertical Lines, Horizontal Lines...
uniform float frequency;    // change the number of stripes / dots

//uniform bool noiseActive;
uniform sampler2D noise;
uniform float noiseFactor;

vec3 testLightPosition = vec3(0, 10, 4);    //(test) light position in world coordinates
uniform bool UseSun;
uniform float SunlightInfluence;
uniform vec3 lightPosition;
uniform vec3 lightColor;

layout (location = 0) out vec4 result;

float computePattern(float var){ 

    // using the gradient to ajust the density of the lines
    float grad = length(vec2(dFdx(var), dFdy(var)));    // dFdx(x), dFdy(x) -> partial derivatives
                                                        // use the gradient's length for rotational invariance
    float logGrad = -log2(grad * 8.0);                  // negated logarithm to decrease the number of stripes if density gets too high
    float intLogGrad = floor(logGrad);                  // find the nearest integer less than or equal to logDp
    float stripes = exp2(intLogGrad);                   // number of stripes = 2^intLogGrad

                                    // sawtooth wave with peaks between -1 and 1
    
    // add adjustable noise to give the lines an unique style
    vec2 ObjPos = vec2 (gl_FragCoord.xy / screenSize) * noiseFactor;
    float noiseTerm = texture(noise, ObjPos).x;
    float sawtooth = fract((var + noiseTerm * 0.1) * stripes * frequency);
    
    // I decided not to use a bool to check if noise is active... Because if you set noiseFactor == 0.0, noise will be inactive anyways
    /* float sawtooth;     
    if (noiseActive){                                                            
        vec2 ObjPos = vec2 (gl_FragCoord.xy / screenSize) * noiseFactor;
        float noiseTerm = texture(noise, ObjPos).x;
        
        sawtooth = fract((var + noiseTerm * 0.1) * stripes * frequency);
    }
    else {
        sawtooth = fract(var * stripes * frequency);  
    } */
    
    float triangle = abs(2.0 * sawtooth - 1.0);                     // triangle wave with values from 1 to 0 to 1
    float transition = logGrad - intLogGrad;                        //soften the edges between frequencies
    triangle = abs((1.0 + transition) * triangle - transition);     //interpolation

    return triangle;
} 


void main (){
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

    float lightIntensity = max(dot(lightVec, normalVec), 0.0);

    float V = ecPos.x;      // vertical stripes
    float H = ecPos.y;      // horizontal stripes

    float triangleV = computePattern(V);
    float triangleH = computePattern(H);

    // simulate lighting -> in lit regions, the stripes get thinner, in dark regions, the strips get wider
    const float edgeW = 0.5;                                    // width of smooth step
    float edge0 = clamp(lightIntensity - edgeW, 0.0, 1.0);
    float edge1 = clamp(lightIntensity, 0.0, 1.0);

    float term;

    if (mode == 0){                         // chess 
        term = triangleV - triangleH;   
    }
    else if (mode == 1){                    // classic Stippling
        term = triangleV * triangleH;
    }
    else if (mode == 2){                    // vertical lines
        term = triangleV;
    }
    else if (mode == 3){                    // horizontal lines
       term = triangleH;   
    }
    else if (mode == 4){                    // plaid
        term = triangleV / triangleH;   
    }
    else {
        term = triangleV + triangleH;   
    }

    float square = 1.0 - smoothstep(edge0, edge1,term);  

    vec4 texColor;
    vec3 diffuseColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize).rgb;

    if (Textured) {
        //get the color from the texture
        texColor = texture(textureDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

    vec3 color = vec3(square);

    if (Colored){           // use colors of the object
        color = color * diffuseColor;
    }

    if (UseSun) {    // add the sunlight to the color
        color = color + lightColor * SunlightInfluence;
    }

    result = vec4(color, 1.0);
}