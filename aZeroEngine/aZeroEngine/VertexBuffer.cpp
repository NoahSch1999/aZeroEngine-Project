#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
}

void VertexBuffer::Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, int _numElements, const std::string& _name)
{
	SetName(_name);
	numVertices = _numElements;
	//sizePerSubresource = _numBytes;

	//Helper::CreateBufferResource(_device, _copyList, mainResource, sizePerSubresource, intermediateResource, sizePerSubresource, _data);
	BufferResource::Init(_device, _copyList, _numBytes, false, _data, false);

	int stride = _numBytes / _numElements;
	view.BufferLocation = gpuAddress;
	view.SizeInBytes = sizePerSubresource;
	view.StrideInBytes = stride;

}
