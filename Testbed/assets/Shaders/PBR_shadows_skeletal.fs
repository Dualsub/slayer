#version 450 core

const float PI = 3.14159265359;
const int numShadowSamples = 3;
#define MAX_POINT_LIGHTS 64 

out vec4 FragColor;

in VertexOutput
{
    vec3 WorldPosition;
	vec2 TexCoord;
    vec3 Normal;
} vs_Input;

in vec3 viewPos;

struct Material {
    sampler2D albedo;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D ao;
};

struct IBL {
    samplerCube irradiance;
    samplerCube prefilter;
    sampler2D brdf;
};

uniform Material material;
uniform IBL ibl;
uniform sampler2D shadowMap;
uniform vec3 lightPos;

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
    mat4 lightSpaceMatrix;
    int numLights;
};


vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normal, vs_Input.TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(vs_Input.WorldPosition);
    vec3 Q2  = dFdy(vs_Input.WorldPosition);
    vec2 st1 = dFdx(vs_Input.TexCoord);
    vec2 st2 = dFdy(vs_Input.TexCoord);

    vec3 N   = normalize(vs_Input.Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

vec3 CalcDirLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic, vec3 F0)
{
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;

    vec3 kS = F;

    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
   return (kD * albedo / PI + specular) * light.color * NdotL;
}  

float CalcShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    float currentDepth = projCoords.z;

    vec3 normal = normalize(vs_Input.Normal);
    vec3 lightDir = normalize(lightPos - vs_Input.WorldPosition);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -numShadowSamples/2; x <= numShadowSamples/2; ++x)
    {
        for(int y = -numShadowSamples/2; y <= numShadowSamples/2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= (numShadowSamples * numShadowSamples);

    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{	
    vec3 albedo = pow(texture(material.albedo, vs_Input.TexCoord).rgb, vec3(2.2));
    float metallic = texture(material.metallic, vs_Input.TexCoord).r;
    float roughness = texture(material.roughness, vs_Input.TexCoord).r;
    float ao = texture(material.ao, vs_Input.TexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - vs_Input.WorldPosition);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    Lo += CalcDirLight(directionalLight, N, V, albedo, roughness, metallic, F0);

    // for(int i = 0; i < min(numLights, MAX_POINT_LIGHTS); i++)
    // {
    //     vec3 L = normalize(pointLights[i].position - vs_Input.WorldPosition);
    //     vec3 H = normalize(V + L);
    //     float distance = length(pointLights[i].position - vs_Input.WorldPosition);
    //     float attenuation = 1.0 / (distance * distance);
    //     vec3 radiance = pointLights[i].color * attenuation;        
   
    //     float NDF = DistributionGGX(N, H, roughness);   
    //     float G = GeometrySmith(N, V, L, roughness);    
    //     vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);        

    //     vec3 numerator = NDF * G * F;
    //     float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    //     vec3 specular = numerator / denominator;
        
    //     vec3 kS = F;
    //     vec3 kD = vec3(1.0) - kS;
    //     kD *= 1.0 - metallic;	                

    //     float NdotL = max(dot(N, L), 0.0);        
    //     Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    // } 

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(ibl.irradiance, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(ibl.prefilter, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(ibl.brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    float shadow = CalcShadow(lightSpaceMatrix * vec4(vs_Input.WorldPosition, 1.0));

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo * (1.0 - shadow);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}

