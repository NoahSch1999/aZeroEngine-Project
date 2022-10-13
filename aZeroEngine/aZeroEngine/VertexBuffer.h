#pragma once
#include "BaseResource.h"
#include "CommandList.h"

class VertexBuffer : public BaseResource
{
public:
	VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride);
	~VertexBuffer();
	ID3D12Resource* uploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW view;
};

