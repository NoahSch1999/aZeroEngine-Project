// Global variable access via cb
cbuffer outlineSettings : register(b0)
{
    float3 mainOutlineColor;
    int outlineThickness;
    float3 secondOutlineColor;
    int renderTargetID;
    int pickingTextureID;
    int numSelections;
};

StructuredBuffer<int> SelectionList : register(t0, space0);

// Thread groups are launched via ::Dispatch(tGroupCountX, tGroupCountY, tGroupCountZ)

// Number of threads per dimension per thread group
// Thread groups consist of n threads. The hardware divides these into warps and each is processed by the multiprocessor in SIMD (32 bit lane for nvidia)
// Each CUDA core processes a thread.
[numthreads(16, 16, 1)]

void main(uint3 dispatchThreadID : SV_DispatchThreadID )
{
    // Frank D. Luna says out of bound reads result in 0 and out of bound writes result in no-ops.
    // So you don't need to check for out of bound threadIDs.
    
    /*
    uint width, height;
    pickingRTV.GetDimensions(width, height);
    
    if (dispatchThreadID.x > width || dispatchThreadID.x < 0 || dispatchThreadID.y > height || dispatchThreadID.y < 0)
        return;
    */
    
    Texture2D<uint> pickingRTV = ResourceDescriptorHeap[pickingTextureID];
    
    RWTexture2D<float4> finalRenderTarget = ResourceDescriptorHeap[renderTargetID];
    
    uint texelID = pickingRTV[dispatchThreadID.xy];
    
    if(texelID != -1)
    {
        for (int i = 0; i < numSelections; i++)
        {
            if(texelID == SelectionList[i])
            {
                float3 outlineColor = secondOutlineColor;
                if (texelID == SelectionList[0])
                {
                    outlineColor = mainOutlineColor;
                }
        
                for (int row = -outlineThickness; row < outlineThickness; row++)
                {
                    for (int column = -outlineThickness; column < outlineThickness; column++)
                    {
                        uint2 texel = uint2(row, column);
                        uint texelIDKernel = pickingRTV[texel + dispatchThreadID.xy];
                        if (texelIDKernel != texelID)
                        {
                            finalRenderTarget[dispatchThreadID.xy] = float4(outlineColor, 1.f);
                            return;
                        }
                    }
                }
                break;
            }
        }
    }
}