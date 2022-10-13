#include "CommandQueue.h"

CommandQueue::CommandQueue(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	D3D12_COMMAND_QUEUE_FLAGS _flags = D3D12_COMMAND_QUEUE_FLAG_NONE)
{
	D3D12_COMMAND_QUEUE_DESC desc{ _type, _prio, _flags, 0 };
	HRESULT hr = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue));
	if (FAILED(hr))
		throw;

	hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
		throw;

	hr = fence->Signal(0);
	if (FAILED(hr))
		throw;

	type = _type;
	nextFenceValue = 0;
	lastReachedValue = 0;
}

CommandQueue::~CommandQueue()
{
	queue->Release();
	fence->Release();
}

int CommandQueue::Execute(CommandList* _cmdLists, int _numLists = 1)
{
	_cmdLists->graphic->Close();
	queue->ExecuteCommandLists(_numLists, &_cmdLists->basic);
	queue->Signal(fence, nextFenceValue);
	return nextFenceValue++;
}

void CommandQueue::Flush(UINT _fenceValue, CommandAllocator* _allocator, ID3D12GraphicsCommandList* _cmdList)
{
	if(FenceReached(_fenceValue))
	{
		_allocator->allocator->Reset();
		_cmdList->Reset(_allocator->allocator, nullptr);
		return;
	}
	fence->SetEventOnCompletion(_fenceValue, eventHandle);
	WaitForSingleObjectEx(eventHandle, INFINITE, false);
	lastReachedValue = _fenceValue;
	_allocator->allocator->Reset();
	_cmdList->Reset(_allocator->allocator, nullptr);
}

void CommandQueue::Flush()
{
	if (FenceReached(nextFenceValue))
	{
		return;
	}
	fence->SetEventOnCompletion(nextFenceValue, eventHandle);
	WaitForSingleObjectEx(eventHandle, INFINITE, false);
	lastReachedValue = nextFenceValue;
}

bool CommandQueue::FenceReached(UINT _fenceValue)
{
	if (_fenceValue > lastReachedValue)
	{
		lastReachedValue = PollCurrentFenceValue();
	}

	return _fenceValue <= lastReachedValue;
}

int CommandQueue::PollCurrentFenceValue()
{
	if (lastReachedValue <= fence->GetCompletedValue())
	{
		lastReachedValue = (UINT)fence->GetCompletedValue();
	}
	return lastReachedValue;
}
