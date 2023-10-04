#version 450 core

#define MAX_INSTANCES 128

layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;

layout(std140, binding = 3) uniform Instance {
    mat4 transformMatrices[MAX_INSTANCES];
    int animInstanceIDs[MAX_INSTANCES];
};

void main()
{
    mat4 transformMatrix = transformMatrices[gl_InstanceID];
    gl_Position = lightSpaceMatrix * transformMatrix * vec4(aPos, 1.0);
}