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

uniform int mode;           // Stippling, Vertical Lines, Horizontal Lines...
uniform float frequency;    // change the number of stripes / dots

uniform bool noiseActive;
uniform float noise;

vec3 lightPosition = vec3(0, 10, 4);            //(test) light position in world coordinates

layout (location = 0) out vec4 result;


float computeStripes(float var){ 

    // using the gradient to ajust the density of the lines
    float grad = length(vec2(dFdx(var), dFdy(var)));    // dFdx(x), dFdy(x) -> partial derivatives
                                                        // use the gradient's length for rotational invariance
    float logGrad = -log2(grad * 8.0);                  // negated logarithm to decrease the number of stripes if density gets too high
    float intLogGrad = floor(logGrad);                  // find the nearest integer less than or equal to logDp
    float stripes = exp2(intLogGrad);                   // number of stripes = 2^intLogGrad

    float sawtooth;                                     // sawtooth wave with peaks between -1 and 1
    if (noiseActive){
        sawtooth = fract((var + noise * 0.1) * stripes * frequency);
    }
    else {
        sawtooth = fract(var * stripes * frequency);  
    }
    
    float triangle = abs(2.0 * sawtooth - 1.0);         // triangle wave with values from 1 to 0 to 1
    
    float transition = logGrad - intLogGrad;            //soften the edges between frequencies

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

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    //vec3 lightPosition = vec3(gl_LightSource[0].position);

    vec3 lightVec = normalize(lightPosition - ecPos);
    vec3 normalVec = normalize(tNorm);
    vec3 viewVec = normalize(-ecPos);

    float lightIntensity = max(dot(lightVec, normalVec), 0.0);

    float V = ecPos.x;      // vertical stripes
    float H = ecPos.y;      // horizontal stripes

    float triangleV = computeStripes(V);
    float triangleH = computeStripes(H);

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

    vec3 color;

    if (Colored){                                       // use colors of the object
        color = diffuseColor * vec3(square);
    }
    else { 
        color = vec3(square);
    } 

    result = vec4(color, 1.0);
}