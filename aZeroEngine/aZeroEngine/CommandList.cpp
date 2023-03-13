#include "CommandList.h"

CommandList::CommandList(ID3D12Device* _device, CommandAllocator& _allocator)
{
	Init(_device, _allocator);
}

CommandList::~CommandList()
{
	basic->Release();
}

void CommandList::Init(ID3D12Device* _device, CommandAllocator& _allocator)
{
	type = _allocator.GetType();
	HRESULT hr = _device->CreateCommandList(0, type, _allocator.GetAllocator(), nullptr, IID_PPV_ARGS(&basic));
	if (FAILED(hr))
		throw;
	graphic = (ID3D12GraphicsCommandList*)basic;
}
