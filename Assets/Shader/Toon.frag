#version 450 core

layout(location = 0) out vec4 FragColor;
uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat4 projectionMatrix;

uniform vec3 lightPosition = vec3(0, 10, 4); //light position in world coordinates

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform vec3 cameraPosition;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

uniform bool Textured;

uniform vec3 vColor;        //color of the vertex

// Sobel
mat3 sx = mat3( 
    1.0, 2.0, 1.0, 
    0.0, 0.0, 0.0, 
   -1.0, -2.0, -1.0 
);
mat3 sy = mat3( 
    1.0, 0.0, -1.0, 
    2.0, 0.0, -2.0, 
    1.0, 0.0, -1.0 
);

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

void main(void) {
	// Discard if no geometry is present
	if(texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
		discard;
	}

	// Deferred shading
	vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
	vec3 position = vPosition.xyz;

	vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;
    //vec3 lightPosition = vec3(gl_LightSource[0].position);
	
	vec3 viewVec = normalize(cameraPosition.xyz - position);

	vec3 lightVec = normalize(lightPosition.xyz - position);

	vec3 reflecVec = reflect(-lightVec, tNorm);

    vec4 texColor;
    vec3 color = texture(colorDiffuse, gl_FragCoord.xy / screenSize).rgb;
    
    if (Textured) {
        //get the color from the texture
        texColor = texture(textureDiffuse, gl_FragCoord.xy / screenSize);
    	color = texColor.rgb;
    }
	
//-------------------------------------------------------------------------------------

	vec3 lightColor = vec3(1.0f);
	
	//ambient lighting
	float Kamb = 0.7;
	vec3 ambient = Kamb * lightColor;
	
	//diffused lighting
	const int levels = 6;						//number of levels for diffuse color
	const float scaleFactor = 1.0 / levels;
	float Kdiff = 0.5; 
	float diff = max(dot(tNorm, lightVec), 0.0); 	//to clamp between 0 and 1
	vec3 diffuse = Kdiff * floor(diff * levels) * scaleFactor * lightColor;  	//light intensity * cos
	
	//specular lighting
	float Kspec = 0.5f;
	float spec = pow(max(dot(viewVec, reflecVec), 0.0), 16);
	vec3 specular = vec3(0.0, 0.0, 0.0);
	if( dot(lightVec,tNorm) > 0.0)
	{
		specular = Kspec*spec*lightColor;
	}
	float spec_thresh = 0.4; //set threshold for specular lighting
	float specMask = (pow(dot(reflecVec, tNorm), Kspec) > spec_thresh) ? 1 : 0;  //limit specular

	//Get outline
	float edge_thresh = 0.0; //set threshold for edge detection
	float visiblity = dot(viewVec, tNorm);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1; 	//Black color if dot product is smaller than 0.2 else keep the same colors
	
	
	//final color for object	
	vec3 final_color= (ambient + diffuse + specular * specMask)* color;
	
	// ---------- SOBEL ------------
	vec4 n[9];
	make_kernel( n, depth, gl_FragCoord.xy / screenSize);//gl_TexCoord[0].st );

	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

	vec3 outputColor3;

	vec3 sobelout = 1.0 - sobel.xyz;
	if ((sobelout.x + sobelout.y + sobelout.z) >= 2.99f) {
		outputColor3 = final_color;
	}
	else {
		outputColor3 = vec3(0.0);
	}

	FragColor = vec4(outputColor3, 1.0 );

}
