#define myTex2DSpace space1
#define myTexCubeSpace space2

Texture2D Texture2DTable[] : register(t0, myTex2DSpace);

cbuffer PBRConstants : register(b0)
{
    int albedoMapIndex;
    int roughnessMapIndex;
    int metallicMapIndex;
    float roughnessFactor;
    float metallicFactor;
};

cbuffer Camera : register(b2)
{
    float4 cameraDir;
    float4 cameraPos;
}

SamplerState basicSampler : register(s0);

struct FragmentInput
{
    float4 position : SV_Position;
    float3 worldPosition : WORLDPOSITION;
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

StructuredBuffer<PointLight> pointLights : register(t0, space0);

StructuredBuffer<DirectionalLight> dLights : register(t2, space0);

/*
_normalDotHalf Is the dot product of the fragment normal and the half vector
_roughness Is the roughness sampled from the roughness texture (or taken from the roughness factor) with a power of 4
*/
float REITZGGXNormalDistFunc(float _normalDotHalf, float _roughness)
{
    float alpha2 = _roughness;
    
    float d = _normalDotHalf * _normalDotHalf * (alpha2 - 1.f) + 1.f;
    
    float ggxdistrib = alpha2 / (3.14f * d * d);
    
    return ggxdistrib;
}

/*
_dotProdNVL Is the dot product of the fragment normal and the half vector OR the fragment normal and the light vector
_roughnessFactor Is the roughness factor sampled from the roughness texture (or taken from the roughness factor)
*/
float SMITHGeometryFunc(float _dotProdNVL, float _roughnessFactor)
{
    float k = (_roughnessFactor + 1.f) * (_roughnessFactor + 1.f) / 8.f;
    
    float denom = _dotProdNVL * (1.f - k) + k;
    
    return _dotProdNVL / denom;
}

/*
_viewDotHalf Is the dot product of the view vector and the half vector
_albedoColor Is the color of the material sampled from the albedo texture
_metallicFactor Is the metallic factor sampled fromt he metallic texture (or taken from the metallic factor)
*/
float3 SCHLICKFresnelFunc(float _viewDotHalf, float3 _albedoColor, float _metallicFactor)
{
    float3 F0 = float3(0.4f, 0.4f, 0.4f) /** _metallicFactor*/;
    if(_metallicFactor >= 0.5)
    {
        F0 = _albedoColor;
    }
    
    return F0 + (1.f - F0) * pow(clamp(1.f - _viewDotHalf, 0.f, 1.f), 5.f);
}

float3 CalcPBRPointLight(PointLight _pLight, float3 _albedoColor, float3 _fragNormal, float3 _fragWorldPos, float _metallic, float _roughness)
{
    float3 lightIntensity = _pLight.color * _pLight.intensity;
    
    float3 lightVec = _pLight.position.xyz - _fragWorldPos;
    float ligthDistance = length(lightVec);
    lightVec = normalize(lightVec);
    
    lightIntensity /= (ligthDistance * ligthDistance);
    
    float3 viewVec = normalize(cameraPos.xyz - _fragWorldPos);
    float3 halfVec = normalize(viewVec + lightVec);
    
    float nDotH = max(dot(_fragNormal, halfVec), 0.f);
    float vDotH = max(dot(viewVec, halfVec), 0.f);
    float nDotL = max(dot(_fragNormal, lightVec), 0.f);
    float nDotV = max(dot(_fragNormal, viewVec), 0.f);
    
    float3 fresnel = SCHLICKFresnelFunc(vDotH, _albedoColor, _metallic);
    
    float3 kS = fresnel;
    float3 kD = 1.f - kS;
    
    float3 specBRDFNom = REITZGGXNormalDistFunc(nDotH, _roughness * _roughness * _roughness * _roughness) 
                                                * fresnel 
                                                * SMITHGeometryFunc(nDotL, _roughness) * SMITHGeometryFunc(nDotV, _roughness);
    
    float specBRDFDenom = 4.f * nDotV * nDotL + 0.0001f;
    
    float3 specBRDF = specBRDFNom / specBRDFDenom;
    
    float3 fLambert = { 0.f, 0.f, 0.f };
    
    if(_metallic < 0.5f)
    {
        fLambert = _albedoColor/* * _metallic*/;
    }
    
    float3 diffuseBRDF = kD * fLambert / 3.14f;
    
    return (diffuseBRDF + specBRDF) * lightIntensity * nDotL;
}

float4 main(FragmentInput input) : SV_Target
{
    float3 fragNDCPos = input.position;
    float3 fragNormal = input.normal;
    float3 fragWorldPos = input.worldPosition;
    
    float4 albedoColor = Texture2DTable[albedoMapIndex].Sample(basicSampler, input.uv);
    
    float roughness = roughnessFactor;
    if(roughnessMapIndex != -1)
    {
        roughness = Texture2DTable[roughnessMapIndex].Sample(basicSampler, input.uv).x;
    }
    
    float metallic = metallicFactor;
    if (metallicMapIndex != -1)
    {
        metallic = Texture2DTable[metallicMapIndex].Sample(basicSampler, input.uv).x;
    }
    
    float3 totalLighting = float3(0.f, 0.f, 0.f);
    for (int i = 0; i < numPointLights; i++)
    {
        totalLighting += CalcPBRPointLight(pointLights[i], albedoColor.xyz, fragNormal, fragWorldPos, metallic, roughness);
    }
    
    totalLighting = totalLighting / (totalLighting + float3(1.f, 1.f, 1.f));
    //return float4(totalLighting, 1.f);
    return float4(pow(totalLighting, float3(1.f / 2.2f, 1.f / 2.2f, 1.f / 2.2f)), 1.f);
}