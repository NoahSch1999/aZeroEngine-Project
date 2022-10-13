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

void CommandAllocator::InitCommandList(ID3D12Device* _device, CommandList* _cmdList)
{
	HRESULT hr = _device->CreateCommandList(0, type, allocator, nullptr, IID_PPV_ARGS(&_cmdList->basic));
	if (FAILED(hr))
		throw;
	_cmdList->type = type;
	_cmdList->graphic = (ID3D12GraphicsCommandList*)_cmdList->basic;
}
