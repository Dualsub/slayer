#version 450 core

out vec4 FragColor;

in vec2 outTexCoord;

uniform sampler2D inTexture;

void main()
{
    FragColor = texture(inTexture, outTexCoord);
}