#version 450

layout (location = 0) in vec2 pos;
layout (location = 2) in vec2 aTexCoords;

void main()
{
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}

/*#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec3 normal;
out vec3 position;
out vec2 texCoord;
out float nDotVP;

void main()
{
    // Light
	vec3 lightPos = vec3(0.2f, -0.2f, 1.2f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

    mat4 normalMatrix =  transpose ( inverse ( viewMatrix * modelMatrix));
    normal = (normalize( normalMatrix * vec4(aNormal.xyz, 0.0))).xyz;

    position = (viewMatrix * modelMatrix * vec4(aPos.xyz, 1.0)).xyz;

    texCoord = aTexCoord;

    float depth = ( length( aPos.xyz ) / 90. );
    depth = .5 + .5 * depth;

    nDotVP = max( 0, dot(normal, normalize(vec3(lightPos))));

    gl_Position = projectionMatrix * vec4(position.xyz, 1.0);
}
*/