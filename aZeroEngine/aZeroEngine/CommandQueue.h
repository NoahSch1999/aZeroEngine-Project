#pragma once
#include "CommandAllocator.h"

class CommandQueue
{
private:
	bool FenceReached(UINT _fenceValue);
	int PollCurrentFenceValue();
public:
	ID3D12CommandQueue* queue;
	ID3D12Fence* fence;
	UINT nextFenceValue;
	UINT lastReachedValue;
	D3D12_COMMAND_LIST_TYPE type;
	HANDLE eventHandle;

	CommandQueue(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio, D3D12_COMMAND_QUEUE_FLAGS _flags);
	~CommandQueue();

	int Execute(CommandList* _cmdLists, int _numLists);
	void Flush(UINT _fenceValue, CommandAllocator* _allocator, ID3D12GraphicsCommandList* _cmdList);
	void Flush();
};

