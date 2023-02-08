#version 450

uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat4 projectionMatrix;            //world coordinates to camera coordinate
uniform vec3 cameraPosition;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;
uniform vec2 screenSize;
uniform sampler2D heightMap;
uniform bool displayNormals;

uniform bool colored;
//uniform bool Textured;

out vec4 FragColor;

void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord)
{
	float width = screenSize.x;
	float height = screenSize.y;
	float w = 1.0 / width;
	float h = 1.0 / height;

	n[0] = texture(tex, coord + vec2( -w, -h));
	n[1] = texture(tex, coord + vec2(0.0, -h));
	n[2] = texture(tex, coord + vec2(  w, -h));
	n[3] = texture(tex, coord + vec2( -w, 0.0));
	n[4] = texture(tex, coord);
	n[5] = texture(tex, coord + vec2(  w, 0.0));
	n[6] = texture(tex, coord + vec2( -w, h));
	n[7] = texture(tex, coord + vec2(0.0, h));
	n[8] = texture(tex, coord + vec2(  w, h));
}

void main(void) 
{	
	// Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

    // Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec3 position = vPosition.xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

	vec3 eye = normalize(cameraPosition-position);

	// -----------------------------------------------------------------------

	vec4 n[9];
	make_kernel( n, normals, gl_FragCoord.xy / screenSize);//gl_TexCoord[0].st );

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));
	
	float strength = length(sobel.xyz);//sqrt(sobel_edge_h.x*sobel_edge_h.x + sobel_edge_h.y*sobel_edge_h.y + sobel_edge_h.z*sobel_edge_h.z + sobel_edge_v.x*sobel_edge_v.x + sobel_edge_v.y*sobel_edge_v.y + sobel_edge_v.z*sobel_edge_v.z);
	vec3 color = vec3(1.0);
	if (colored)
	{
		color = texture(colorDiffuse, gl_FragCoord.xy / screenSize).xyz;
	}

	if (displayNormals)
    {
        color = texture(normals, gl_FragCoord.xy / screenSize).xyz;
    }

	FragColor = vec4(color * strength, 1.0);
} 