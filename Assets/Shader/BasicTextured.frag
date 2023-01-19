#version 450 core
out vec4 FragColor;

in vec2 uvCoords;

uniform sampler2D textureDiffuse;

void main()
{    
    // Discard pixels according to alpha channel
    vec4 texColor = texture(textureDiffuse, uvCoords);
    if(texColor.a < 1.0)
        discard;
    FragColor = texColor;
}