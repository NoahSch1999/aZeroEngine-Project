cbuffer PerDraw : register(b0)
{
    int worldIndex;
}

cbuffer Camera : register(b1)
{
    float4x4 camera;
}

StructuredBuffer<float4x4> transforms : register(t0, space0);

struct VertexIn
{
    float3 position : POSITION;
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