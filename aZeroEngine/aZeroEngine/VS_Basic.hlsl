//#pragma pack_matrix(row_major)

cbuffer World : register(b0)
{
    float4x4 world;
}

cbuffer Camera : register(b1)
{
    float4x4 view;
    float4x4 proj;
}

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 normal : NORMAL;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.position = mul(world, float4(input.position, 1.f));
    output.position = mul(view, float4(output.position));
    output.position = mul(proj, float4(output.position));
    
    //output.position = mul(float4(input.position, 1.f), world);
    //output.position = mul(float4(output.position), view);
    //output.position = mul(float4(output.position), proj);
    
    output.uv = input.uv;
    float4 normal = normalize(mul(world, float4(input.normal, 0.f)));
    output.normal = normal.xyz;
    
    return output;
}