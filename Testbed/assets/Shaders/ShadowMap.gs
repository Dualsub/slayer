#version 450 core

#define MAX_POINT_LIGHTS 64 
#define NUM_SHADOW_CASCADES 4

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

struct PointLight {    
    vec3 position;
    vec3 color;
};  

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

//Lights
layout(std140, binding = 1) uniform Lights { 
    DirectionalLight directionalLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    mat4 lightSpaceMatrices[NUM_SHADOW_CASCADES];
    float cascadeEnds[NUM_SHADOW_CASCADES];
    int numLights;
};

void main()
{          
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = 
            lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
} 