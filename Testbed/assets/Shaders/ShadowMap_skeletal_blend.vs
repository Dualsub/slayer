// Skeletal VS
#version 450 core

#define MAX_BONES 100
#define MAX_WEIGHTS 4
#define MAX_ANIMATIONS 8

layout (location = 0) in vec3 aPos;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 transformMatrix;

uniform sampler2D animTex;
uniform ivec2 frames;
uniform float time;
uniform int parents[MAX_BONES];

layout(std140, binding = 2) uniform Bones { 
    mat4 invBindPose[MAX_BONES];
};

vec3 QMulV(vec4 q, vec3 v) {
    return q.xyz * 2.0f * dot(q.xyz, v) +
           v * (q.w * q.w - dot(q.xyz, q.xyz)) +
           cross(q.xyz, v) * 2.0f * q.w;
}

mat4 GetPose(int joint) {

    int x_now = frames.x;
    int x_next = frames.y;
    int y_pos = joint * 3;
    
    vec4 pos0 = texelFetch(animTex, ivec2(x_now, y_pos + 0), 0);
    vec4 rot0 = texelFetch(animTex, ivec2(x_now, y_pos + 1), 0);
    vec4 scl0 = texelFetch(animTex, ivec2(x_now, y_pos + 2), 0);

    vec4 pos1 = texelFetch(animTex, ivec2(x_next, y_pos + 0), 0);
    vec4 rot1 = texelFetch(animTex, ivec2(x_next, y_pos + 1), 0);
    vec4 scl1 = texelFetch(animTex, ivec2(x_next, y_pos + 2), 0);

    if (dot(rot0, rot1) < 0.0) { rot1 *= -1.0; }

    vec4 position = mix(pos0, pos1, time);
    vec4 rotation = normalize(mix(rot0, rot1, time));
    vec4 scale = mix(scl0, scl1, time);

    vec3 xBasis = QMulV(rotation, vec3(scale.x, 0, 0));
    vec3 yBasis = QMulV(rotation, vec3(0, scale.y, 0));
    vec3 zBasis = QMulV(rotation, vec3(0, 0, scale.z));

    return mat4(
        xBasis.x, xBasis.y, xBasis.z, 0.0,
        yBasis.x, yBasis.y, yBasis.z, 0.0,
        zBasis.x, zBasis.y, zBasis.z, 0.0,
        position.x, position.y, position.z, 1.0
    );
}

void main()
{
    mat4 boneMatrix = mat4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
		int boneID = aBoneIDs[i];
        if (boneID == -1) continue;
		float weight = aWeights[i];
		boneMatrix += (GetPose(boneID) * invBindPose[boneID]) * weight;
	}

    gl_Position = lightSpaceMatrix * transformMatrix * boneMatrix * vec4(aPos, 1.0);
}