#include "CommandAllocator.h"

CommandAllocator::CommandAllocator(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type)
{
	HRESULT hr = _device->CreateCommandAllocator(_type, IID_PPV_ARGS(&allocator));
	if (FAILED(hr))
		throw;

	type = _type;
}

CommandAllocator::~CommandAllocator()
{
	allocator->Release();
}
