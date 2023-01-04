//shader for Gooch Shading
//Gooch shading: non-realistic rendering
//Designed by Amy & Bruce Gooch

#version 330

layout (location = 0) in vec3 vPos;         //position of the vertex
vec4 vPosition = vec4(vPos.x, vPos.y, vPos.z, 1.0); 

layout (location = 1) in vec3 vNormal;      //normal of the vertex

uniform mat4 modelMatrix;           //model coordinates to world coordinates
uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat3 normalMatrix;          //normal matrix = top left 3x3 portion of the transpose inverse of the modelview matrix
uniform mat4 projectionMatrix;      //camera coordinates to homogenous coordinates
//uniform mat3 normalMatrix;          //normal matrix = top left 3x3 portion of the transpose inverse of the modelview matrix

vec3 lightPosition = vec3(0, 10, 4);

out vec3 reflecVec;      //reflection vector
out vec3 viewVec;        //view vector
out float NdotL;         //lighting value

void main()
{
    vec4 vertex = vec4(viewMatrix * modelMatrix * vPosition);
    vec3 ecPos = vertex.xyz;

    vec3 tNorm = normalize(normalMatrix * vNormal);

    //vec3 lightPosition = vec3(gl_LightSource[0].position);
    vec3 lightVec = normalize(lightPosition - ecPos);

    reflecVec = normalize(reflect(-lightVec, tNorm));
    viewVec = normalize(-ecPos);
    NdotL = (dot(lightVec, tNorm) + 1.0) * 0.5;

    gl_Position = projectionMatrix * vertex;
}