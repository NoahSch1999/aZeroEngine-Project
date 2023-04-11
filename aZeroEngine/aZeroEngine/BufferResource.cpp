#include "BufferResource.h"

void BufferResource::Init(ID3D12Device* _device, CommandList& _copyList, int _width, bool _trippleBuffered, void* _data, bool _dynamic)
{
	isDynamic = _dynamic;
	sizePerSubresource = _width;
	isTrippleBuffered = _trippleBuffered;

	int uploadWidth = _width;
	if (isTrippleBuffered)
		uploadWidth *= 3;

	if (!isDynamic)
	{
		isTrippleBuffered = false;
		uploadWidth = _width;
	}

	if (_data)
	{
		Helper::CreateBufferResource(_device, _copyList, gpuOnlyResource, sizePerSubresource, uploadResource, uploadWidth, _data);

	}
	else
	{
		std::unique_ptr<char[]>tempData = std::make_unique<char[]>(uploadWidth);
		ZeroMemory(tempData.get(), uploadWidth);
		Helper::CreateBufferResource(_device, _copyList, gpuOnlyResource, sizePerSubresource, uploadResource, uploadWidth, tempData.get());
	}

	if (isDynamic)
		uploadResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));


	gpuAddress = gpuOnlyResource->GetGPUVirtualAddress();
}
