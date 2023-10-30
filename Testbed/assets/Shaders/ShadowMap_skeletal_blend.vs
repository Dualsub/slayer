// ShadowMap VS
#version 450 core

#define MAX_SKELETONS 4
#define MAX_INSTANCES 128
#define MAX_BONES 96

#define MAX_WEIGHTS 4

layout (location = 0) in vec3 aPos;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

uniform mat4 lightSpaceMatrix;

struct InstanceData {
    mat4 transformMatrix;
    int animInstanceID;
};

layout(std430, binding = 3) buffer Instance {
    InstanceData instances[];
}; 

layout(std140, binding = 2) uniform Bones { 
    mat4 invBindPose[MAX_BONES];
};

uniform sampler2D boneTransformTex;

mat4 GetPose(int boneID, int instanceID) {
    int x_pos = instanceID;
    int y_pos = boneID * 4;

    mat4 boneMat = mat4(0.0);
    boneMat[0] = texelFetch(boneTransformTex, ivec2(x_pos, y_pos + 0), 0);
    boneMat[1] = texelFetch(boneTransformTex, ivec2(x_pos, y_pos + 1), 0);
    boneMat[2] = texelFetch(boneTransformTex, ivec2(x_pos, y_pos + 2), 0);
    boneMat[3] = texelFetch(boneTransformTex, ivec2(x_pos, y_pos + 3), 0);

    return boneMat * invBindPose[boneID];
}

void main()
{
    InstanceData instance = instances[gl_InstanceID];
    int animInstanceID = instance.animInstanceID;
    mat4 boneMatrix = mat4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
		int boneID = aBoneIDs[i];
        
        if (boneID == -1) {
            continue;
        } 

		boneMatrix += GetPose(boneID, animInstanceID) * aWeights[i];
	}

    gl_Position = instance.transformMatrix * boneMatrix * vec4(aPos, 1.0);
}