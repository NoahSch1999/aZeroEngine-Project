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
    float2 uv : UV;
    float3 normal : NORMAL;
};

float4 main(FragmentInput input) : SV_Target
{
    float4 diffuse = Texture2DTable[diffuseIndex].Sample(basicSampler, input.uv);
	return diffuse;
}