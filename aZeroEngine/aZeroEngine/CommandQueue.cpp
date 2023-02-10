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

void CommandQueue::Init(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
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

int CommandQueue::Execute(CommandList& _cmdList)
{
	_cmdList.CloseGraphic();
	ID3D12CommandList* cmdLists = { _cmdList.GetBaseList() };
	queue->ExecuteCommandLists(1, &cmdLists);
	queue->Signal(fence, nextFenceValue);
	return nextFenceValue++;
}

void CommandQueue::Flush(UINT _fenceValue, CommandAllocator& _allocator, CommandList& _cmdList)
{
	if(FenceReached(_fenceValue))
	{
		_allocator.Reset();
		_cmdList.ResetGraphic(_allocator);
		return;
	}
	fence->SetEventOnCompletion(_fenceValue, eventHandle);
	WaitForSingleObjectEx(eventHandle, INFINITE, false);
	lastReachedValue = _fenceValue;
	_allocator.Reset();
	_cmdList.ResetGraphic(_allocator);
}

void CommandQueue::Flush(UINT _fenceValue)
{
	if (FenceReached(_fenceValue))
	{
		return;
	}
	fence->SetEventOnCompletion(_fenceValue, eventHandle);
	WaitForSingleObjectEx(eventHandle, INFINITE, false);
	lastReachedValue = _fenceValue;
}

bool CommandQueue::FenceReached(UINT _fenceValue)
{
	if (_fenceValue > lastReachedValue)
	{
		lastReachedValue = PollCurrentFenceValue();
	}

	if (lastReachedValue >= _fenceValue)
		return true;
	
	return false;
}

int CommandQueue::PollCurrentFenceValue()
{
	if (lastReachedValue <= fence->GetCompletedValue())
	{
		lastReachedValue = (UINT)fence->GetCompletedValue();
	}
	return lastReachedValue;
}
