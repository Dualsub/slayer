#version 450 core

#define MAX_INSTANCES 128

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out VertexOutput
{
    vec3 WorldPosition;
	vec2 TexCoord;
    vec3 Normal;
} vs_Output;

out vec3 viewPos;

layout(std140, binding = 3) uniform Instance {
    mat4 transformMatrices[MAX_INSTANCES];
    int animInstanceIDs[MAX_INSTANCES];
};

layout(std140, binding = 0) uniform Camera { 
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 position;
};

void main()
{
    mat4 transformMatrix = transformMatrices[gl_InstanceID];
	vs_Output.TexCoord = aTexCoord;
	vs_Output.Normal = mat3(transformMatrix) * aNormal;
    vs_Output.WorldPosition = vec3(transformMatrix * vec4(aPos, 1.0));
    viewPos = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(vs_Output.WorldPosition, 1.0);
}