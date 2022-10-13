#pragma once
#include "directx/d3dx12.h"

class CommandList
{
public:
	CommandList() = default;
	~CommandList();
	ID3D12CommandList* basic;
	ID3D12GraphicsCommandList* graphic;
	D3D12_COMMAND_LIST_TYPE type;
};

