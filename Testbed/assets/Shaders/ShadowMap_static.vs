#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 transformMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * transformMatrix * vec4(aPos, 1.0);
}