#pragma once
#include "CommandAllocator.h"

class CommandList
{
public:
	CommandList() = default;
	CommandList(ID3D12Device* _device, CommandAllocator* _allocator);
	~CommandList();
	ID3D12CommandList* basic;
	ID3D12GraphicsCommandList* graphic;
	D3D12_COMMAND_LIST_TYPE type;

	void Init(ID3D12Device* _device, CommandAllocator* _allocator);
};

