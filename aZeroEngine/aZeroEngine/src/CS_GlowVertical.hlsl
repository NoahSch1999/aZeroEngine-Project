cbuffer ConstantData : register(b0)
{
    int srcTextureID;
    int dstTextureID;
    int kernelRadius;
};

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Texture2D<float4> srcTexture = ResourceDescriptorHeap[srcTextureID];
    RWTexture2D<float4> dstTexture = ResourceDescriptorHeap[dstTextureID];
    
    float weights[99] =
    {
        0, 0, 0.000001, 0.000001, 0.000002, 0.000003, 0.000004, 0.000006, 0.000009, 0.000013, 0.00002, 0.000029, 0.000043, 0.000061, 0.000088
        , 0.000124, 0.000173, 0.000239, 0.000328, 0.000445, 0.000597, 0.000794, 0.001045, 0.001362, 0.001757, 0.002244, 0.002838, 0.003553, 0.004405
        , 0.005406, 0.006569, 0.007902, 0.009412, 0.011099, 0.012959, 0.014979, 0.017142, 0.019422, 0.021787, 0.024197, 0.026606, 0.028965, 0.031219
        , 0.033314, 0.035196, 0.036814, 0.038124, 0.039089, 0.039679, 0.039878, 0.039679, 0.039089, 0.038124, 0.036814, 0.035196, 0.033314, 0.031219
        , 0.028965, 0.026606, 0.024197, 0.021787, 0.019422, 0.017142, 0.014979, 0.012959, 0.011099, 0.009412, 0.007902, 0.006569, 0.005406, 0.004405
        , 0.003553, 0.002838, 0.002244, 0.001757, 0.001362, 0.001045, 0.000794, 0.000597, 0.000445, 0.000328, 0.000239, 0.000173, 0.000124, 0.000088
        , 0.000061, 0.000043, 0.000029, 0.00002, 0.000013, 0.000009, 0.000006, 0.000004, 0.000003, 0.000002, 0.000001, 0.000001, 0, 0
    };
    
    int numWeights = 99;
    int blurRadius = kernelRadius;
    
    float3 blurColor = float3(0, 0, 0);
    
    for (int i = -blurRadius; i <= blurRadius; i++)
    {
        int k = dispatchThreadID.y + i;
        
        blurColor += srcTexture[uint2(dispatchThreadID.x, k)] * weights[numWeights / 2 + i];

    }

    dstTexture[dispatchThreadID.xy].xyz += blurColor;
}