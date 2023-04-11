#include "ConstantBuffer.h"

void ConstantBuffer::Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, bool _dynamic, int _trippleBuffered)
{
	int width = (_numBytes + 255) & ~255;
	BufferResource::Init(_device, _copyList, width, _trippleBuffered, _data, _dynamic);

	//sizePerSubresource = (_numBytes + 255) & ~255; // size per buffer subresource
	//isTrippleBuffered = _tripple;
	//isDynamic = _dynamic;
	//mainResourceState = D3D12_RESOURCE_STATE_COMMON;
	//intermediateResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	//int uploadWidth = sizePerSubresource;
	//if (isTrippleBuffered)
	//	uploadWidth *= 3;

	//if (_data == nullptr)
	//{
	//	char* tempData = new char[_numBytes];
	//	ZeroMemory(tempData, _numBytes);
	//	Helper::CreateBufferResource(_device, _copyList, mainResource, sizePerSubresource, intermediateResource, uploadWidth, tempData);
	//	delete[] tempData;
	//}
	//else
	//{
	//	Helper::CreateBufferResource(_device, _copyList, mainResource, sizePerSubresource, intermediateResource, uploadWidth, _data);
	//}

	//if(isDynamic)
	//	intermediateResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));

	//gpuAddress = mainResource->GetGPUVirtualAddress();
}
