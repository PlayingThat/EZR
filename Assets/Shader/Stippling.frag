#version 450 core

out vec4 FragColor;
uniform vec3 cameraPosition;

uniform vec2 screenSize;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D depth;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;

uniform sampler2D stipp1;
uniform sampler2D stipp2;
uniform sampler2D stipp3;
uniform sampler2D stipp4;
uniform sampler2D stipp5;
uniform sampler2D stipp6;
uniform sampler2D paper;

vec3 lightPosition = vec3(0, 10, 4);

vec4 shade() {
	vec4 c;

	vec2 nUV = vec2(mod(gl_FragCoord.x, screenSize.x) / screenSize.x, mod(gl_FragCoord.y, screenSize.y) / screenSize.y);
	vec4 dst = vec4(texture(paper, nUV).xyz, 1.);

	vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
    vec3 position = vPosition.xyz;

	vec3 viewVec = normalize(cameraPosition.xyz - position);

	vec3 lightPos = normalize(lightPosition - position);

    vec3 normal = texture(normals, gl_FragCoord.xy / screenSize).xyz;

	vec2 texCoord = (vPosition.xyz / vPosition.w).xy;

	float nDotVP = max( 0, dot(normal, normalize(vec3(viewVec))));

	float diffuse = nDotVP;
	float specular = 0.8f;
	float ambient = 1.0f;

	vec3 r = -reflect(lightPos, normal);
	r = normalize(r);
	vec3 v = -position.xyz;
	v = normalize(v);
	float nDotHV = max(0., dot(r, v));

	int shininess = 20;
	float ambientWeight = 0.0;
	float diffuseWeight = 1.0;
	float rimWeight = 1.0;
	float specularWeight = 1.0;

	//int diffuse = 100, specular: 100, rim: 46
	if(nDotVP != 0.)
		specular = pow(nDotHV, shininess);
	float rim = max(0., abs(dot(normal, normalize(-position.xyz))));

	float shading = ambientWeight * ambient + diffuseWeight * diffuse + rimWeight * rim + specularWeight * specular;

	float step = 1. / 6.;
	if(shading <= step) {
		c = mix(texture(stipp6, texCoord), texture(stipp5, texCoord), 6. * shading); //texture(stipp6, texCoord);
	}
	if(shading > step && shading <= 2. * step) {
		c = mix(texture(stipp5, texCoord), texture(stipp4, texCoord), 6. * (shading - step)); //texture(stipp5, texCoord);//
	}
	if(shading > 2. * step && shading <= 3. * step) {
		c = mix(texture(stipp4, texCoord), texture(stipp3, texCoord), 6. * (shading - 2. * step)); //texture(stipp4, texCoord);//
	}
	if(shading > 3. * step && shading <= 4. * step) {
		c = mix(texture(stipp3, texCoord), texture(stipp2, texCoord), 6. * (shading - 3. * step)); // texture(stipp3, texCoord);//
	}
	if(shading > 4. * step && shading <= 5. * step) {
		c = mix(texture(stipp2, texCoord), texture(stipp1, texCoord), 6. * (shading - 4. * step)); //texture(stipp2, texCoord);//
	}
	if(shading > 5. * step) {
		c = mix(texture(stipp1, texCoord), vec4(1.), 6. * (shading - 5. * step)); //texture(stipp1, texCoord);//
	}

	vec4 inkColor = vec4(0.0);
	vec4 src = mix(mix(inkColor, vec4(1.0), c.r), c, .5);
	c = src * dst;
	
	return c;
}

void main(void) {
	// Discard if no geometry is present
    if (texture(depth, gl_FragCoord.xy / screenSize).x == 1.0f) {
        discard;
    }
	vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
	vec2 texCoord = (vPosition.xyz / vPosition.w).xy;

	//final color for object
	vec4 final_color = texture(stipp1, normalize(texCoord));//shade();
    FragColor = vec4(final_color.xyz, 1.0f);
}