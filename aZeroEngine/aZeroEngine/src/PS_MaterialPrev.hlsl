#define myTex2DSpace space1
#define myTexCubeSpace space2

cbuffer PerDrawConstants : register(b0)
{
    int diffuseIndex;
    float3 ambientAbsorbation;
    float3 specularAbsorbation;
    float specularExponent;
};

struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3x3 TBN : TBN;
};

float4 main(FragmentInput input) : SV_Target
{
    SamplerState basicSampler = SamplerDescriptorHeap[0];
    Texture2D texture = ResourceDescriptorHeap[diffuseIndex];
    float4 diffuse = texture.Sample(basicSampler, input.uv);
	return diffuse;
}