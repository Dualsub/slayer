#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 outColor;

layout(std140, binding = 0) uniform Camera {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
};

void main()
{
	outColor = aColor;
	gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
}

