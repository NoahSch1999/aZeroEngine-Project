#define myTex2DSpace space1
#define myTexCubeSpace space2

Texture2D Texture2DTable[] : register(t0, myTex2DSpace);

cbuffer PerDrawConstants : register(b0)
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
    float2 uv : UV;
    float3 normal : NORMAL;
};

// Lights
cbuffer PerDrawConstants : register(b1)
{
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;
};

struct PointLight
{
    float4 position;
    float3 color;
    float range;
};

StructuredBuffer<PointLight>pointLights : register(t0, space0);
StructuredBuffer<int>pointLightIndices : register(t1, space0);

float4 main(FragmentInput input) : SV_Target
{

    float4 output = float4(0,0,0,1);
    float4 color = Texture2DTable[diffuseIndex].Sample(basicSampler, input.uv);
    float3 finalDiffuse = float3(0,0,0);

    for (int i = 0; i < numPointLights; i++)
    {
        PointLight currentLight = pointLights[pointLightIndices[i]];

        float4 lightDirection = float4(currentLight.position.xyz, 1.f) - float4(input.worldPosition, 1.f);
        lightDirection.w = 0.f;

        if (currentLight.range < length(lightDirection))
            continue;
        
        normalize(lightDirection);

        float4 normal = normalize(float4(input.normal, 0.f));
      // return normal;

        float lightAffectDir = max(dot(normalize(lightDirection), normal), 0);
        
        

        float3 tempColor = currentLight.color * lightAffectDir;

        color += float4(tempColor, 0.f);
    }
    return color;

}