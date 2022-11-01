#include "CommandList.h"

CommandList::CommandList(ID3D12Device* _device, CommandAllocator* _allocator)
{
	Init(_device, _allocator);
}

CommandList::~CommandList()
{
	basic->Release();
}

void CommandList::Init(ID3D12Device* _device, CommandAllocator* _allocator)
{
	HRESULT hr = _device->CreateCommandList(0, type, _allocator->allocator, nullptr, IID_PPV_ARGS(&basic));
	if (FAILED(hr))
		throw;
	type = _allocator->type;
	graphic = (ID3D12GraphicsCommandList*)basic;
}
