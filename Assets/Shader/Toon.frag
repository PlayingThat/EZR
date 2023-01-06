#version 450 core

out vec4 FragColor;
uniform vec3 camPosition;

in vec3 normal;
in vec3 position;
in vec2 texCoord;

void main(void) 
{
	// Light
	vec3 lightPos = vec3(0.2f, -0.2f, 1.2f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

	vec3 lightDir = normalize(lightPos - position);  
	vec3 viewDir = normalize(camPosition - position);

	//ambient lighting
	float Kamb = 0.7;
	vec3 ambient = Kamb * lightColor;
	
	//diffused lighting
	const int levels = 5;						//number of levels for diffuse color
	const float scaleFactor = 1.0 / levels;
	float Kdiff= 0.5; 
	float diff= max(dot(normal, lightDir), 0.0); 	//to clamp between 0 and 1
	vec3 diffuse= Kdiff * floor(diff * levels) * scaleFactor * lightColor;  	//light intensity * cos
	
	//specular lighting
	float Kspec = 0.5f;
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular = vec3(0.0, 0.0, 0.0);
	if( dot(lightDir,normal) > 0.0)
	{
		specular= Kspec*spec*lightColor;
	}
	float spec_thresh = 0.4; //set threshold for specular lighting
	float specMask = (pow(dot(reflectDir, normal), Kspec) > spec_thresh) ? 1 : 0;  //limit specular

	//Get outline
	float edge_thresh = 0.0; //set threshold for edge detection
	float visiblity = dot(viewDir, normal);

	float edge_detection = (visiblity > edge_thresh) ? 0 : 1; 	//Black color if dot product is smaller than 0.2 else keep the same colors
	
	
	//final color for object
	vec3 color = vec3(0.0, 0.0, 1.0);
	
	vec3 final_color;
	if(edge_detection == 0){
		final_color = (ambient + diffuse + specular * specMask)* color;
	}else{
		float scale_origin = 0.5;
		float scale = scale_origin + edge_thresh;
		float factor = (visiblity + scale_origin)/scale;
		final_color = factor * ambient * color;
	}
	FragColor = vec4(final_color, 1.0f);
}