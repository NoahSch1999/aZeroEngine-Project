cbuffer PerDraw : register(b0)
{
    int worldIndex;
}

StructuredBuffer<float4x4> transforms : register(t1, space0);

cbuffer Camera : register(b1)
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

struct VertexOut
{
    float4 position : SV_Position;
};

VertexOut main(VertexIn input)
{
    float4x4 world = transforms[worldIndex];
    VertexOut output;
    output.position = mul(world, float4(input.position, 1.f));
    output.position = mul(camera, output.position);
	return output;
}