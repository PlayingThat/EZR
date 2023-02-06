#version 450

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D uvs;
uniform sampler2D depth;
uniform sampler2D tangents;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform sampler2D stipp1;
uniform sampler2D stipp2;
uniform sampler2D stipp3;
uniform sampler2D stipp4;
uniform sampler2D stipp5;
uniform sampler2D stipp6;
uniform sampler2D paper;

uniform vec2 screenSize;

uniform bool textured;      // If true, the shader will be applied on the textured object

uniform vec3 lightPosition = vec3(0, 10, 4);            //(test) light position in world coordinates

out vec4 FragColor;

void main()
{
    // Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec3 position = vPosition.xyz;

    // calculate curve
    vec3 curve = vec3(sin(position.x), cos(position.y), position.y);

    // ligthvector
    vec3 lightVec = normalize(lightPosition - position);

    // normal
    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;
    vec3 normal = normalize(tNorm);
    
    // texture coords
    vec2 uv = texture(uvs, gl_FragCoord.xy / screenSize).xy;

//-------------------------------------------------------
    float LightIntensity = max(dot(lightVec, normal), 0.0);

    float V = curve.y; //(gl_FragCoord.xy / screenSize).y;

    float square;

    //float width = length(vec2(dFdx(V), dFdy(V))); //gradient
    //float edge = width * 32.0;

    float lineAmount = 16.0;
    // sawtooth hives us value from 0-1 and repeats
    float sawtooth = fract(V * lineAmount);
    // trianlge switches between 1 and 0
    float triangle = abs(2.0 * sawtooth - 1.0);

    // fake shading
    float edgew = 0.1;            // width of smooth step

    float edge0  = clamp(LightIntensity - edgew, 0.0, 0.5);
    float edge1  = clamp(LightIntensity, 0.0, 1.0);

    float threshold = 0.4;
    // avoid aliasing
    square = 1.0 - smoothstep(edge0, edge1, triangle); //smoothstep(0.5 - edge, 0.5 + edge, triangle);
    // modify threshold to change stripe size
 /*     if (triangle <= threshold)
        square = 0;
    else
        square = 1;
*/
    FragColor = vec4(vec3(square), 1.0);

    /*
    //--- add 3d noise texture to make it look handdrawn ---
    //float noise = texture(noise, uv).x;
    // sawtooth hives us value from 0-1 and repeats
    float sawtooth = fract((V * 0.1)) * frequency * stripes); //fract((V + noise * 0.1) * frequency * stripes);
    // trianlge switches between 1 and 0
    float triangle = abs(2.0 * sawtooth - 1.0);

    // adjust line width
    float transition = logdp - ilogdp;

    // taper ends
    triangle = abs((1.0 + transition) * triangle - transition);

    const float edgew = 0.3;            // width of smooth step

    float edge0  = clamp(LightIntensity - edgew, 0.0, 1.0);
    float edge1  = clamp(LightIntensity, 0.0, 1.0);
    //float square = 1.0 - smoothstep(edge0, edge1, triangle);

    float square;
    if (triangle <= 0.5)
        square = 0;
    else
        square = 1;

    FragColor = vec4(vec3(square), 1.0);
    */
}
