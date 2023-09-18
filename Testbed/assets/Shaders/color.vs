#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 outTexCoord;

uniform mat4 transformMatrix;

layout(std140, binding = 0) uniform Camera { 
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 position;
};

void main()
{
    gl_Position = projectionMatrix * viewMatrix * transformMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    outTexCoord=aTexCoord;
};