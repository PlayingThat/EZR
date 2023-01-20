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

vec3 lightPosition = vec3(0, 10, 4);            //(test) light position in world coordinates

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
    vec3 lightVec = normalize(lightPosition);// - position);

    // normal
    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;
    vec3 normal = normalize(tNorm);
    
    // texture coords
    vec2 uv = texture(uvs, gl_FragCoord.xy / screenSize).xy;

//-------------------------------------------------------
    float LightIntensity = max(dot(lightVec, normal), 0.0);

    float V = curve.y;

    float dp = length(vec2(dFdx(V), dFdy(V)));
    float logdp = -log2(dp * 8.0);
    float ilogdp = floor(logdp);
    float stripes = exp2(ilogdp);
    float frequency = exp2(ilogdp);

    //--- add 3d noise texture to make it look handdrawn ---
    //float noise = texture(noise, uv).x;
    float sawtooth = fract((V * 0.1) * frequency * stripes); //fract((V + noise * 0.1) * frequency * stripes);
    float triangle = abs(2.0 * sawtooth - 1.0);

    // adjust line width
    float transition = logdp - ilogdp;

    // taper ends
    triangle = abs((1.0 + transition) * triangle - transition);

    const float edgew = 0.3;            // width of smooth step

    float edge0  = clamp(LightIntensity - edgew, 0.0, 1.0);
    float edge1  = clamp(LightIntensity, 0.0, 1.0);
    float square = 1.0 - smoothstep(edge0, edge1, triangle);
    
    FragColor = vec4(vec3(square), 1.0);
}
