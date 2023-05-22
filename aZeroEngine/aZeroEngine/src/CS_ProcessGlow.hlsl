cbuffer ConstantData : register(b0)
{
    int srcTextureID;
    int dstTextureID;
    int horizontal;
    int kernelWidth;
    int kernelHeight;
};

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Texture2D<float4> srcTexture = ResourceDescriptorHeap[srcTextureID];
    RWTexture2D<float4> dstTexture = ResourceDescriptorHeap[dstTextureID];
    
    if(horizontal == 1)
    {
        int startOffsetX = dispatchThreadID.x - (kernelWidth / 2);
        int endOffsetX = dispatchThreadID.x + (kernelWidth / 2);
        float3 totalColor = float3(0, 0, 0);
        for (int i = startOffsetX; i < endOffsetX + 1; i++)
        {
            uint2 texelIndex = uint2(i, dispatchThreadID.y);
            totalColor += srcTexture[texelIndex].xyz;
        }
        float3 finalColor = totalColor / kernelWidth;
        dstTexture[dispatchThreadID.xy].xyz = finalColor;
    }
    else
    {
        int startOffsetY = dispatchThreadID.y - (kernelHeight / 2);
        int endOffsetY = dispatchThreadID.y + (kernelHeight / 2);
        float3 totalColor = float3(0, 0, 0);
        for (int i = startOffsetY; i < endOffsetY + 1; i++)
        {
            uint2 texelIndex = uint2(dispatchThreadID.x, i);
            totalColor += srcTexture[texelIndex].xyz;
        }
        float3 finalColor = totalColor / kernelHeight;
        dstTexture[dispatchThreadID.xy].xyz += finalColor;
    }
}