// Skeletal VS
#version 450 core

#define MAX_SKELETONS 4
#define MAX_INSTANCES 128
#define MAX_BONES 96

#define MAX_WEIGHTS 4
#define MAX_ANIMATIONS 8

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

layout(std140, binding = 0) uniform Camera { 
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 position;
};

struct AnimationState {
    ivec2 frames;
    float time;
};

layout(std140, binding = 3) uniform Instance {
    mat4 transformMatrices[MAX_INSTANCES];
    int animInstanceIDs[MAX_INSTANCES];
};

uniform sampler2D animTex;

layout(std140, binding = 2) uniform Bones { 
    mat4 invBindPose[MAX_BONES];
};

vec3 QMulV(vec4 q, vec3 v) {
    return q.xyz * 2.0f * dot(q.xyz, v) +
           v * (q.w * q.w - dot(q.xyz, q.xyz)) +
           cross(q.xyz, v) * 2.0f * q.w;
}

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
    int animInstanceID = animInstanceIDs[gl_InstanceID];
    mat4 boneMatrix = mat4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
		int boneID = aBoneIDs[i];
        
        if (boneID == -1) {
            continue;
        } 

		boneMatrix += GetPose(boneID, animInstanceID) * aWeights[i];
	}

    mat4 transformMatrix = transformMatrices[gl_InstanceID];
	vs_Output.TexCoord = aTexCoord;
	vs_Output.Normal = vec3(transformMatrix * boneMatrix * vec4(aNormal, 0.0));
    vs_Output.WorldPosition = vec3(transformMatrix * boneMatrix * vec4(aPos, 1.0));
    viewPos = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(vs_Output.WorldPosition, 1.0);
}