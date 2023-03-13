cbuffer World : register(b0)
{
    float4x4 world;
}

cbuffer LightMatrix : register(b1)
{
    float4x4 lightMatrix;
}

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

float4 main(VertexIn input) : SV_POSITION
{
    float4 output;
    output = mul(world, float4(input.position, 1.f));
    output = mul(lightMatrix, float4(output));
    
    
    return output;
}