#include "ConstantBuffer.h"

void ConstantBuffer::Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, bool _dynamic, int _trippleBuffered)
{
	int width = (_numBytes + 255) & ~255;
	BufferResource::Init(_device, _copyList, width, _trippleBuffered, _data, _dynamic);
}
