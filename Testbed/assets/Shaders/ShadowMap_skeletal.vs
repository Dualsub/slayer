#version 450 core

#define MAX_BONES 100
#define MAX_WEIGHTS 4

layout (location = 0) in vec3 aPos;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 transformMatrix;

layout(std140, binding = 2) uniform Bones { 
    mat4 boneMatrices[MAX_BONES];
};

void main()
{
    mat4 boneMatrix = boneMatrices[aBoneIDs[0]] * aWeights[0];
    boneMatrix += boneMatrices[aBoneIDs[1]] * aWeights[1];
    boneMatrix += boneMatrices[aBoneIDs[2]] * aWeights[2];
    boneMatrix += boneMatrices[aBoneIDs[3]] * aWeights[3];

    gl_Position = lightSpaceMatrix * transformMatrix * boneMatrix * vec4(aPos, 1.0);
}