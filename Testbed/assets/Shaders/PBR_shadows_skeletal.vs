// Skeletal VS
#version 450 core

#define MAX_BONES 100
#define MAX_WEIGHTS 4

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

out VertexOutput
{
    vec3 WorldPosition;
	vec2 TexCoord;
    vec3 Normal;
} vs_Output;

out vec3 viewPos;

uniform mat4 transformMatrix;

layout(std140, binding = 0) uniform Camera { 
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 position;
};

layout(std140, binding = 2) uniform Bones { 
    mat4 boneMatrices[MAX_BONES];
};

void main()
{
    mat4 boneMatrix = boneMatrices[aBoneIDs[0]] * aWeights[0];
    boneMatrix += boneMatrices[aBoneIDs[1]] * aWeights[1];
    boneMatrix += boneMatrices[aBoneIDs[2]] * aWeights[2];
    boneMatrix += boneMatrices[aBoneIDs[3]] * aWeights[3];

	vs_Output.TexCoord = aTexCoord;
	vs_Output.Normal = vec3(transformMatrix * boneMatrix * vec4(aNormal, 0.0));
    vs_Output.WorldPosition = vec3(transformMatrix * boneMatrix * vec4(aPos, 1.0));
    viewPos = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(vs_Output.WorldPosition, 1.0);
}