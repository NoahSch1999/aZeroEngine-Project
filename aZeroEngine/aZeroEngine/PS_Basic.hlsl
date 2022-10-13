Texture2D test : register(t0);
Texture2D testx : register(t1);

SamplerState basicSampler : register(s0);

cbuffer buf : register(b0)
{
    float4 x;
}

struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 normal : NORMAL;
};

float4 main(FragmentInput input) : SV_Target
{
    return float4(input.normal, 1.f);
}