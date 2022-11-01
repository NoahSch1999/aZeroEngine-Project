#pragma once
#include "directx/d3dx12.h"

class CommandAllocator
{
private:

public:
	D3D12_COMMAND_LIST_TYPE type;
	ID3D12CommandAllocator* allocator;

	CommandAllocator() = default;
	CommandAllocator(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type);
	~CommandAllocator();
};

