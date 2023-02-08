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

	std::vector<ID3D12Resource*>tempResources; // add intermediate resources to this and clear it once after sync
};

