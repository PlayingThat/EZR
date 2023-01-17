#version 450 core

out vec4 FragColor;
uniform vec3 cameraPosition;

uniform vec2 screenSize;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;
uniform sampler2D uvs;

vec3 lightPosition = vec3(0, 10, 4);

void main() { 
	// Discard if no geometry is present
	if(texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
		discard;
	}

	float offset = 5.0;
	float lum_threshold_1 = 1.0;
	float lum_threshold_2 = 0.75;
	float lum_threshold_3 = 0.5;
	float lum_threshold_4 = 0.3;

	vec2 uv = (gl_FragCoord.xy / screenSize).xy;

	vec3 tc = vec3(1.0, 0.0, 0.0);

	float lum = length(texture(uvs, uv).xyz);
	tc = vec3(1.0, 1.0, 1.0);

	if(lum < lum_threshold_1) {
		if(mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0)
			tc = vec3(0.0, 0.0, 0.0);
	}

	if(lum < lum_threshold_2) {
		if(mod(gl_FragCoord.x - gl_FragCoord.y, 10.0) == 0.0)
			tc = vec3(0.0, 0.0, 0.0);
	}

	if(lum < lum_threshold_3) {
		if(mod(gl_FragCoord.x + gl_FragCoord.y - offset, 10.0) == 0.0)
			tc = vec3(0.0, 0.0, 0.0);
	}

	if(lum < lum_threshold_4) {
		if(mod(gl_FragCoord.x - gl_FragCoord.y - offset, 10.0) == 0.0)
			tc = vec3(0.0, 0.0, 0.0);
	}

	FragColor = vec4(tc, 1.0);
}