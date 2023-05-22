#define myTex2DSpace space1
#define myTexCubeSpace space2

struct MaterialConstants
{
    int albedoMapIndex;
    int roughnessMapIndex;
    int metallicMapIndex;
    int normalMapIndex;
    float roughnessFactor;
    float metallicFactor;
    
    int enableTransparency;
    int transparencyMapIndex;
    float transparencyFactor;
    
    int enableGlow;
    int glowMapIndex;
    float glowIntensity;
    float3 glowFullColor;
};

StructuredBuffer<MaterialConstants> materials : register(t1, space0);

// Lights
cbuffer LightConstants : register(b1)
{
    int numPointLights;
    int numSpotLights;
    int pad;
};

cbuffer Camera : register(b2)
{
    float4 cameraDir;
    float4 cameraPos;
}

cbuffer ShadowMap : register(b3)
{
    int shadowMapIndex;
}

cbuffer PerDrawConstants : register(b4)
{
    int receiveShadows;
    int materialIndex;
    int pickingID;
}

cbuffer DirectionalLight : register(b5)
{
    float4 direction;
    float3 color;
    float pad2;
};

struct FragmentInput
{
    float4 position : SV_Position;
    float3 worldPosition : WORLDPOSITION;
    float4 lightPosition : LIGHTPOSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3x3 TBN : TBN;
};

struct PointLight
{
    float3 color;
    float intensity;
    float3 position;
    float range;
};

//struct DirectionalLight
//{
//    float4 direction;
//    float3 color;
//    float pad;
//};

StructuredBuffer<PointLight> pointLights : register(t0, space0);

/*
_normalDotHalf Is the dot product of the fragment normal and the half vector
_roughness Is the roughness sampled from the roughness texture (or taken from the roughness factor) with a power of 4
*/
float REITZGGXNormalDistFunc(float _normalDotHalf, float _roughness)
{
    float d = _normalDotHalf * _normalDotHalf * (_roughness - 1.f) + 1.f;
    d = max(d, 0.0001); // To avoid division by zero which on nvidia results in infinite with the same sign value
    float ggxdistrib = _roughness / (3.14f * d * d);
    
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
    
    float nDotL = dot(_fragNormal, lightVec);
    
    // Remove lights that are behind the face or parallell (angle is 90deg or more from face normal and incidence vector)
    if(nDotL <= 0)
        return float3(0.f, 0.f, 0.f);
    
    nDotL = max(nDotL, 0.f);
    
    float3 viewVec = normalize(cameraPos.xyz - _fragWorldPos);
    float3 halfVec = normalize(viewVec + lightVec);
    
    float nDotH = max(dot(_fragNormal, halfVec), 0.f);
    float vDotH = max(dot(viewVec, halfVec), 0.f);
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

struct Output
{
    float4 fragmentColor : SV_TARGET0;
    int pickingOutput : SV_TARGET1;
    float4 glowOutput : SV_TARGET2;
};

Output main(FragmentInput input)
{
    MaterialConstants matData = materials[materialIndex];
    
    SamplerState basicSampler = SamplerDescriptorHeap[0];
    float3 fragNDCPos = input.position;
    float3 fragWorldPos = input.worldPosition;
    
    float3 fragNormal = input.normal;
    
    if (matData.normalMapIndex != -1)
    {
        Texture2D<float4> normalTexture = ResourceDescriptorHeap[matData.normalMapIndex];
        fragNormal = normalTexture.Sample(basicSampler, input.uv);
        fragNormal = normalize(fragNormal * 2.0 - 1.0);
        fragNormal = normalize(mul(fragNormal, input.TBN));
    }
    
    Texture2D<float4> albedoTexture = ResourceDescriptorHeap[matData.albedoMapIndex];
    float4 albedoColor = albedoTexture.Sample(basicSampler, input.uv);
    albedoColor = pow(albedoColor, 2.2f);
    
    float roughness = matData.roughnessFactor;
    if (matData.roughnessMapIndex != -1)
    {
        Texture2D<float4> roughnessTexture = ResourceDescriptorHeap[matData.roughnessMapIndex];
        roughness = roughnessTexture.Sample(basicSampler, input.uv).x;
    }
    
    float metallic = matData.metallicFactor;
    if (matData.metallicMapIndex != -1)
    {
        Texture2D<float4> metallicTexture = ResourceDescriptorHeap[matData.metallicMapIndex];
        metallic = metallicTexture.Sample(basicSampler, input.uv).x;
    }
    
    float3 totalLighting = float3(0.f, 0.f, 0.f);
    for (int i = 0; i < numPointLights; i++)
    {
        totalLighting += CalcPBRPointLight(pointLights[i], albedoColor.xyz, fragNormal, fragWorldPos, metallic, roughness);
    }
    
    totalLighting = totalLighting / (totalLighting + float3(1.f, 1.f, 1.f));
    
    float shadow = 1.f;
    if (/*receiveShadows == 1*/false) // If receive shadows is true
    {
        float3 projCoords = input.lightPosition.xyz / input.lightPosition.w;
        float2 sampleIndices = float2(0.5f * projCoords.x + 0.5f, -0.5f * projCoords.y + 0.5f);
        
        SamplerState shadowSampler = SamplerDescriptorHeap[1];
        Texture2D shadowMap = ResourceDescriptorHeap[shadowMapIndex];
        
        float depth = shadowMap.Sample(shadowSampler, sampleIndices).r;
        shadow = (depth + 0.005f < projCoords.z) ? 0.0f : 1.0f;
    
        if (sampleIndices.x > 1.0f || sampleIndices.x < 0.0f ||
        sampleIndices.y > 1.0f || sampleIndices.y < 0.0f ||
        projCoords.z > 1.0f || projCoords.z < 0.0f)
            shadow = 1.0f;
    
        if (shadow == 0.f)
            shadow += 0.4f;
    }
    
    totalLighting *= shadow;
    
    Output output;
    output.fragmentColor = float4(totalLighting, 1);
    output.pickingOutput = pickingID;
    output.glowOutput = float4(0.f, 0.f, 0.f, 1.f);
    
    if (matData.enableGlow)
    {
        if (matData.glowMapIndex != -1)
        {
            Texture2D glowMap = ResourceDescriptorHeap[matData.glowMapIndex];
            output.glowOutput = glowMap.Sample(basicSampler, input.uv) * float4(matData.glowFullColor, 1.f) * matData.glowIntensity;
        }
        else
        {
            output.glowOutput = float4(matData.glowFullColor, 1.f) * matData.glowIntensity;
        }
    }
    
    return output;
}