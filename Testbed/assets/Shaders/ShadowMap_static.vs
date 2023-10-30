#version 450 core

#define MAX_INSTANCES 128

layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;

struct InstanceData {
    mat4 transformMatrix;
    int animInstanceID;
};

layout(std430, binding = 3) buffer Instance {
    InstanceData instances[];
}; 

void main()
{
    InstanceData instance = instances[gl_InstanceID];
    gl_Position = instance.transformMatrix * vec4(aPos, 1.0);
}