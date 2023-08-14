cbuffer pickingData : register(b0)
{
    int pickingTextureIndex;
    int xTexel;
    int yTexel;
};

RWStructuredBuffer<int> pickingResult : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Texture2DMS<int> pickingRTV = ResourceDescriptorHeap[pickingTextureIndex];
    
    pickingResult[0] = pickingRTV[uint2(xTexel, yTexel)].x;
}