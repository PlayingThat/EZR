#version 450 core

layout (location = 0) out vec4 FragColor;
uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat4 projectionMatrix;

vec3 lightPosition = vec3(0, 10, 4); //light position in world coordinates

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

uniform bool Textured;

uniform vec3 vColor;        //color of the vertex

void main(void) 
{
	// Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }

	// Deferred shading (usually in vertex shader)
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec4 vertex = vPosition;
    vec3 position = vertex.xyz;

    vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;

    //vec3 lightPosition = vec3(gl_LightSource[0].position);
	vec4 lightPositionSpace = vec4(lightPosition, 1.0f);
    vec3 lightVec = normalize(lightPositionSpace.xyz - position);

    vec3 reflecVec = normalize(reflect(-lightVec, tNorm));
    vec3 viewVec = normalize(-position);

    vec4 texColor;
    vec3 diffuseColor = vec3(0.0f, 1.0f, 0.2f);
    
    if (Textured) {
        //get the color from the texture
        texColor = texture(colorDiffuse, gl_FragCoord.xy / screenSize);
        diffuseColor = texColor.rgb;
    }
	
//-------------------------------------------------------------------------------------

	vec3 lightColor = vec3(1.0f, 1.0f, 0.2f);
	
	//ambient lighting
	float Kamb = 0.7;
	vec3 ambient = Kamb * lightColor;
	
	//diffused lighting
	const int levels = 5;						//number of levels for diffuse color
	const float scaleFactor = 1.0 / levels;
	float Kdiff= 0.5; 
	float diff= max(dot(tNorm, lightVec), 0.0); 	//to clamp between 0 and 1
	vec3 diffuse= Kdiff * floor(diff * levels) * scaleFactor * lightColor;  	//light intensity * cos
	
	//specular lighting
	float Kspec = 0.5f;
	float spec = pow(max(dot(viewVec, reflecVec), 0.0), 16);
	vec3 specular = vec3(0.0, 0.0, 0.0);
	if( dot(lightVec,tNorm) > 0.0)
	{
		specular= Kspec*spec*lightColor;
	}
	float spec_thresh = 0.4; //set threshold for specular lighting
	float specMask = (pow(dot(reflecVec, tNorm), Kspec) > spec_thresh) ? 1 : 0;  //limit specular

	//Get outline
	float edge_thresh = 0.0; //set threshold for edge detection
	float visiblity = dot(viewVec, tNorm);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1; 	//Black color if dot product is smaller than 0.2 else keep the same colors
	
	vec3 final_color;
	if(edge_detection == 0){
		final_color = (ambient + diffuse + specular * specMask)* diffuseColor;
	}else{
		float scale_origin = 0.5;
		float scale = scale_origin + edge_thresh;
		float factor = (visiblity + scale_origin)/scale;
		final_color = 1 * ambient * diffuseColor;
	}
	// FragColor = vec4(final_color, 1.0f);
	FragColor = vec4(final_color, 1.0f);
}
