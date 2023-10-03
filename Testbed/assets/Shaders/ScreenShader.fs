#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float exposure;
uniform sampler2D screenTexture;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
}

