#version 450 core
out vec4 FragColor;

in vec2 uvCoords;

uniform sampler2D diffuseTexture;

void main()
{    
    // Discard pixels according to alpha channel
    vec4 texColor = texture(diffuseTexture, uvCoords);
    if(texColor.a < 1.0)
        discard;
    FragColor = texColor;
}