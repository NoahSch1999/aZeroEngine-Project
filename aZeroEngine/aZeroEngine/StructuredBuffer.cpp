#include "StructuredBuffer.h"

D3D12_GPU_VIRTUAL_ADDRESS StructuredBuffer::GetGPUAddress(int _frameIndex)
{
	return gpuAddress + sizePerSubresource * _frameIndex;
}
