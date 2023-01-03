#version 330

uniform vec3 vColor;        //color of the vertex

float DiffuseCool = 0.3;
float DiffuseWarm = 0.3;
vec3 CoolColor = vec3(0, 0, 0.6);    //blue color
vec3 WarmColor = vec3(0.6, 0, 0);    //red color

in vec3 reflecVec;
in vec3 viewVec;
in float NdotL;

out vec4 result;

void main()
{
    //combine cool / warm color with vertex color
    vec3 cool = min(CoolColor + DiffuseCool * vColor, 1.0);
    vec3 warm = min(WarmColor + DiffuseWarm * vColor, 1.0);

    //interpolate using the dot product of normal & lightvector
    vec3 final = mix(cool, warm, NdotL);
 
    //set the specular highlight using the reflected lightvector & viewvector
    vec3 nRefl = normalize(reflecVec);
    vec3 nView = normalize(viewVec);
    float spec = pow(max(dot(nRefl, nView), 0.0), 32.0);
 
    result = vec4(min(final + spec, 1.0), 1.0);
}
