#include "StructuredBuffer.h"

void StructuredBuffer::Init(ID3D12Device* _device, CommandList& _copyList, void* _data, int _numBytes, int _numElements, bool _dynamic, bool _trippleBuffered)
{
	numElements = _numElements;
	sizeOfElement = _numBytes / _numElements;

	BufferResource::Init(_device, _copyList, _numBytes, _trippleBuffered, _data, _dynamic);
}
