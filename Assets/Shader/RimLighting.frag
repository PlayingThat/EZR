#version 450

uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat4 projectionMatrix;            //world coordinates to camera coordinate
uniform vec3 cameraPosition;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

uniform bool textured;

uniform vec3 vColor;        //color of the vertex

vec3 lightPosition = vec3(0, 10, 4); //light position in world coordinates

layout (location = 0) out vec4 FragColor;

void main(void) 
{	
	// Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec4 vertex = vPosition;
    vec3 position = (vertex).xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    vec4 texColor;
    vec3 diffuseColor = vColor;

	vec3 eye = normalize(cameraPosition-position);

	// higher "strength" makes the rim lighting weaker (1.0 -> strong rim lighting)
	float strength = 5.0;
	float rim = pow(1 - max(dot(tNorm, eye), 0), strength);

	// final color for object
	if (textured) {
        //get the color from the texture
        texColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

    if (textured) {
        //get the color from the texture
        texColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }

	FragColor = rim * vec4(1.0f) + vec4(diffuseColor, 1.0f);
}