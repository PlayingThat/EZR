// shader for imitating an oil painting

// Based on "Anisotropic Kuwahara Filtering on the GPU"
// by Jan Eric Kyprianidis, Henry Kang, and Jürgen Döllner

//
// Created by jesbu on 07.02.2023.
//

#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform vec2 screenSize;


vec3 testLightPosition = vec3(0, 10, 4);    //(test) light position in world coordinates
uniform bool UseSun;
uniform float SunlightInfluence;
uniform vec3 lightPosition;
uniform vec4 lightColor;

uniform int Radius;             // filter radius
uniform int NumberSubregions;   // classic Kuwahara uses 4 rectangular subregions 
                                // but it would be possible to generalize the algorithm by using a circle
const int NumberElements = 4;

layout (location = 0) out vec4 result;

// Classic Kuwahara divides the filter kernel into 4 rectangular subregions overlapping by 1 pixel
// The filter response is defined by the mean of a subregion with minimum variance
vec4 classicKuwahara()
{   
    // create a variable for storing the result
    vec4 result = vec4(1.0, 1.0, 1.0, 1.0);

    // set up the rectangular subregions
    struct Window {float xStart, xEnd, yStart, yEnd;};
    Window window[4] = Window[4](
        Window(gl_FragCoord.x - Radius, gl_FragCoord.x, gl_FragCoord.y, gl_FragCoord.y + Radius),    // upper left
        Window(gl_FragCoord.x, gl_FragCoord.x + Radius, gl_FragCoord.y, gl_FragCoord.y + Radius),    // upper right
        Window(gl_FragCoord.x, gl_FragCoord.x + Radius, gl_FragCoord.y - Radius, gl_FragCoord.y),    // lower right
        Window(gl_FragCoord.x - Radius, gl_FragCoord.x, gl_FragCoord.y - Radius, gl_FragCoord.y));   // lower left

    // set up the storing variables, initialize them to contain zeros only
    vec3 mean[NumberElements];
    vec3 variance[NumberElements];
    for (int k = 0; k < NumberElements; k++){
        mean[k] = vec3(0.0);
        variance[k] = vec3(0.0);
    }

    //calculate the number of pixels in each subregion -> (r + 1)^2
    int numPix = (Radius + 1)*(Radius +1);      

    // set sigma value for later comparison
    float min_sigma2 = 1e+2; 

    // calculate the mean and variance of each subregion
    // mean = sum of all values / |number of pixels| 
    // variance = (sum of all values - mean)^2 / |number of pixels|
    for (int k = 0; k < NumberElements; k++){

        for (float j = window[k].yStart; j <= window[k].yEnd; j+= 1.0) {
            for (float i = window[k].xStart; i <= window[k].xEnd; i+= 1.0) {
                vec3 color = texture(textureDiffuse, vec2(i, j) / screenSize).rgb; // get the color of the current pixel
                mean[k] += color;              // sum up all pixel colors in the subregion
                variance[k] += color * color;  // sum up all pixels, to the power of 2
            }
        }

        mean[k] = mean[k] / abs(numPix);                        // divide through the number of pixels
        variance[k] = (variance[k] - mean[k]) / abs(numPix);    // substract mean, divide through pixel number

        // output of Kawahara: mean of a subregion with minimum variance
        float sigma2 = variance[k].r + variance[k].g + variance[k].b;
        if (sigma2 < min_sigma2) {
                min_sigma2 = sigma2;
                result = vec4(mean[k], 1.0);
        }
    }   

    return result;
}

// Generalized Kuwahara divides the circular filter kernel into equal subregions overlapping by 1 pixel
// The filter response is defined as a weighted sum of the local averages
// More weight is given to those averages with low standard deviation
void generalizedKuwahara(){
  
}


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

    //set the specular highlight using the reflected lightvector & viewvector
    vec3 nRefl = normalize(reflecVec);
    vec3 nView = normalize(viewVec);
    float spec = pow(max(dot(nRefl, nView), 0.0), 32.0);

    vec4 color = classicKuwahara() + 0.3 * spec;

    result = color;
}
