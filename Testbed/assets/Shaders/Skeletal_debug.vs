#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

layout(std140, binding = 0) uniform Camera { 
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 position;
};

uniform mat4 transformMatrix;

out vec4 color;

vec3 get_bone_color(int boneID)
{
    float r = 0.5 * sin(845716.0 * sin(123657.0 * boneID) ) + 0.5;
    float g = 0.5 * sin(909766.0 * sin(823664.0 * boneID) ) + 0.5;
    float b = 0.5 * sin(477865.0 * sin(275985.0 * boneID) ) + 0.5;
    return vec3(r,g,b);
}

void main()
{
	float w_sum = aWeights.x + aWeights.y + aWeights.z + aWeights.w;
    vec3 bone_color0 = get_bone_color(aBoneIDs.x);
    vec3 bone_color1 = get_bone_color(aBoneIDs.y) * aWeights.y;
    vec3 bone_color2 = get_bone_color(aBoneIDs.z) * aWeights.z;
    vec3 bone_color3 = get_bone_color(aBoneIDs.w) * aWeights.w;

    color = vec4((bone_color0), 1.0);

    gl_Position = projectionMatrix * viewMatrix * transformMatrix * vec4(aPos, 1.0);
}