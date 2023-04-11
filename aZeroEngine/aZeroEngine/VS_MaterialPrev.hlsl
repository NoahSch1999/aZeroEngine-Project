cbuffer World : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3x3 TBN : TBN;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    
    output.position = mul(world, float4(input.position, 1.f));
    output.position = mul(view, float4(output.position));
    output.position = mul(proj, float4(output.position));
    
    output.uv = input.uv;

    float3 normal = normalize(mul(world, float4(input.normal, 0.f))).xyz;

    float3 tangent = normalize(mul(world, float4(input.tangent, 0.f))).xyz;
    float3 bitangent = normalize(cross(tangent, normal));
    
    output.TBN = float3x3(tangent, bitangent, normal);
    output.normal = normal;
    
    return output;
}