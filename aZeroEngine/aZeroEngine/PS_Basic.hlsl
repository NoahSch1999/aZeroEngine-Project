Texture2D test : register(t0);
Texture2D testx : register(t1);

SamplerState StaticSampler : register(s0);

struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 normal : NORMAL;
};

float4 main(FragmentInput input) : SV_Target
{
    return test.Sample(StaticSampler, input.uv);
}