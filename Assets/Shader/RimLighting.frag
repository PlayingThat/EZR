#version 450 core

out vec4 FragColor;
uniform vec3 camPosition;

in vec3 normal;
in vec3 position;

void main(void) 
{	
	vec3 eye = normalize(-position);

	// higher "strength" makes the rim lighting weaker (1.0 -> strong rim lighting)
	float strength = 5.0;
	float rim = pow(1 - max(dot(normal, eye), 0), strength);

	// final color for object
	vec3 color = vec3(0.0, 0.0, 1.0);
	FragColor = rim * vec4(1.0f) + vec4(color, 1.0f);
}