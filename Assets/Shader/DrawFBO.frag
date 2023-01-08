#version 450 core

in vec2 uvCoords;
uniform sampler2D fbo;
uniform vec2 screenSize;

out vec4 FragColor;

void main()
{
    FragColor = texture(fbo, gl_FragCoord.xy / screenSize);
}