#version 330

uniform mat4 viewMatrix;            //world coordinates to camera coordinate

uniform sampler2D positions;
uniform sampler2D normals;

uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

uniform vec3 vColor;        //color of the vertex

float DiffuseCool = 0.3;
float DiffuseWarm = 0.3;
vec3 CoolColor = vec3(0, 0, 0.6);    //blue color
vec3 WarmColor = vec3(0.6, 0, 0);    //red color

vec3 lightPosition = vec3(0, 10, 4); //light position in world coordinates

layout (location = 0) out vec4 result;

void main()
{
    // Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec4 vertex = viewMatrix * vPosition;
    vec3 ecPos = vertex.xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    //vec3 lightPosition = vec3(gl_LightSource[0].position);
    vec3 lightVec = normalize(lightPosition - ecPos);

    vec3 reflecVec = normalize(reflect(-lightVec, tNorm));
    vec3 viewVec = normalize(-ecPos);
    float NdotL = (dot(lightVec, tNorm) + 1.0) * 0.5;



    //combine cool / warm color with vertex color
    vec3 cool = min(CoolColor + DiffuseCool * vColor, 1.0);
    vec3 warm = min(WarmColor + DiffuseWarm * vColor, 1.0);

    //interpolate using the dot product of normal & lightvector
    vec3 final = mix(cool, warm, NdotL);
 
    //set the specular highlight using the reflected lightvector & viewvector
    vec3 nRefl = normalize(reflecVec);
    vec3 nView = normalize(viewVec);
    float spec = pow(max(dot(nRefl, nView), 0.0), 32.0);
 
    // result = vec4(min(final + spec, 1.0), 1.0);
    result = vec4(1,0,0,1);
}
