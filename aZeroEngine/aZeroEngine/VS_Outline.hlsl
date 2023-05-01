cbuffer matrice : register(b0)
{
    float4x4 world;
}

cbuffer cam : register(b1)
{
    float4x4 camera;
}

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

float4 main(VertexIn input) : SV_POSITION
{
    float4 position = mul(world, float4(input.position, 1.f));
    position = mul(camera, position);
    return position;
}