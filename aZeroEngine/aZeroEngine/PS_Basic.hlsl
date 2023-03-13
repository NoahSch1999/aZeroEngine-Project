#define myTex2DSpace space1
#define myTexCubeSpace space2

Texture2D Texture2DTable[] : register(t0, myTex2DSpace);
Texture2D shadowMap : register(t4, space0);

cbuffer PhongMaterialConstants : register(b0)
{
    int diffuseIndex;
    float3 ambientAbsorbation;
    float3 specularAbsorbation;
    float specularExponent;
};

SamplerState basicSampler : register(s0);

struct FragmentInput
{
    float4 position : SV_Position;
    float3 worldPosition : WORLDPOSITION;
    float4 lightPosition : LIGHTPOSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

// Lights
cbuffer LightConstants : register(b1)
{
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;
    int pad;
};

struct PointLight
{
    float3 color;
    float intensity;
    float3 position;
    float range;
};

struct DirectionalLight
{
    float4 direction;
    float3 color;
    float pad;
};

cbuffer CameraDir : register(b2)
{
    float4 cameraDir;
}

cbuffer MeshConstants : register(b3)
{
    float receiveShadows;
}

StructuredBuffer<PointLight>pointLights : register(t0, space0);

StructuredBuffer<DirectionalLight> dLights : register(t2, space0);

float4 main(FragmentInput input) : SV_Target
{   
    float4 diffuseTextureCol = Texture2DTable[diffuseIndex].Sample(basicSampler, input.uv);
    
    float3 diffuseLighting = float3(0.f, 0.f, 0.f);
    float3 specularLighting = float3(0.f, 0.f, 0.f);
    
    for (int i = 0; i < numPointLights; i++)
    {
        PointLight currentLight = pointLights[i];
       
        if (currentLight.intensity < 0.01f)
            continue;
        
        float4 lightToFragment = float4(currentLight.position.xyz, 1.f) - float4(input.worldPosition, 1.f);
        float attenuation = pow(length(lightToFragment), 2) * 1;
        float lightToFragmentDist = length(lightToFragment);
        
        //if (lightToFragmentDist > currentLight.range)
        //    continue;
        
        lightToFragment = normalize(lightToFragment);
        
        float fragNormalDotLightVec = dot(float4(input.normal, 0.f), float4(lightToFragment.xyz, 0.f));
        
        if (fragNormalDotLightVec < 0.f)
            continue;
        
        // Calc diffuse lighting
        float diffuseAngleAff = saturate(fragNormalDotLightVec);
        
        float3 lightDiffuseAffect = saturate(((currentLight.color * currentLight.intensity) * diffuseAngleAff) / attenuation);
        
        diffuseLighting += lightDiffuseAffect;
        
        float3 reflectionVec = normalize(reflect(lightToFragment.xyz, input.normal));
        float reflScalar = max(0, dot(reflectionVec, cameraDir.xyz));
        
        float3 lightSpecularAffect = pow(reflScalar, specularExponent) / attenuation;
        
        specularLighting += saturate(lightSpecularAffect);
    }
    
    float3 dLightColor = float3(0.f, 0.f, 0.f);
    for (int j = 0; j < numDirectionalLights; j++)
    {
        DirectionalLight currentLight = dLights[j];
        
        float4 normal = normalize(float4(input.normal, 0.f));

        float lightAffectDir = max(dot(normalize(currentLight.direction), normal), 0);
        
        if (lightAffectDir < 0)
            continue;
        
        diffuseLighting += saturate(currentLight.color * lightAffectDir);
    }
    
    float shadow = 1.f;
    if (receiveShadows == 1) // If receive shadows is true
    {
    
        float3 projCoords = input.lightPosition.xyz / input.lightPosition.w;
        float2 sampleIndices = float2(0.5f * projCoords.x + 0.5f, -0.5f * projCoords.y + 0.5f);
        float shadowSample = shadowMap.Sample(basicSampler, sampleIndices).x;
        shadow = (shadowSample + 0.005f < projCoords.z) ? 0.0f : 1.0f;
    
        if (sampleIndices.x > 1.0f || sampleIndices.x < 0.0f ||
        sampleIndices.y > 1.0f || sampleIndices.y < 0.0f ||
        projCoords.z > 1.0f || projCoords.z < 0.0f)
            shadow = 1.0f;
    
        if (shadow == 0.f)
            shadow += 0.4f;
    }
    
    float4 finalColor = float4(diffuseTextureCol.xyz + ((diffuseLighting * ambientAbsorbation) + specularLighting * specularAbsorbation) * shadow + 0.2f, 1.f);
    return finalColor;
}