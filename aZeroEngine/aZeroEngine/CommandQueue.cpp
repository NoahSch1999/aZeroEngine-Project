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
	//lastReachedValue = 0;
}



//void CommandQueue::Flush(UINT _fenceValue, CommandAllocator& _allocator, CommandList& _cmdList)
//{
//	if(FenceReached(_fenceValue))
//	{
//		_allocator.Reset();
//		_cmdList.ResetGraphic(_allocator);
//		return;
//	}
//	fence->SetEventOnCompletion(_fenceValue, eventHandle);
//	WaitForSingleObjectEx(eventHandle, INFINITE, false);
//	lastReachedValue = _fenceValue;
//	_allocator.Reset();
//	_cmdList.ResetGraphic(_allocator);
//}

//void CommandQueue::Flush(UINT _fenceValue)
//{
//	if (FenceReached(_fenceValue))
//	{
//		return;
//	}
//	fence->SetEventOnCompletion(_fenceValue, eventHandle);
//	WaitForSingleObjectEx(eventHandle, INFINITE, false);
//	lastReachedValue = _fenceValue;
//}

UINT64 CommandQueue::Execute(CommandList& _cmdList)
{
	// Close the graphics list to enable execution
	_cmdList.CloseGraphic();

	// Execute the command list
	ID3D12CommandList* cmdLists = { _cmdList.GetBaseList() };
	queue->ExecuteCommandLists(1, &cmdLists);

	// Set a command queue signal for the fence and then return it.
	queue->Signal(fence, nextFenceValue);
	return nextFenceValue++; // Postincrements the value. This means that the nextFenceValue that is returned equals nextFenceValue without ++.
}

// Waits for the signaled value returned from CommandQueue::Execute()
void CommandQueue::StallCPU(int _valueToWaitFor)
{
	// Get last fence value reached 
	// The fence is automatically updated whenever the ID3D12CommandQueue reaches a signal command for the fence.
	int currentFenceValue = fence->GetCompletedValue();

	// Check if the fence has reached _valueToWaitFor. _valueToWaitFor has to have been set using ID3D12CommandQueue::Signal()
	// Simply return if the fence value has been reached since nothing on the queue is pending that was executed before _valueToWaitFor was signaled for the fence.
	if (_valueToWaitFor <= currentFenceValue)
	{
		return;
	}
	
	// Set an event handle that will be notified, and thus stop locking, once the fence has reached _valueToWaitFor.
	// Nullptr will result in an INFINITE wait until the fence has been signaled.
	fence->SetEventOnCompletion(_valueToWaitFor, nullptr);

	return;
}

// Waits for the input CommandQueue and its signaled value
void CommandQueue::WaitForOther(CommandQueue& _other, int _valueToWaitFor)
{
	queue->Wait(_other.fence, _valueToWaitFor);
	//_other.WaitForValue(_valueToWaitFor);
}

void CommandQueue::WaitForFence(int _valueToWaitFor)
{
	queue->Wait(fence, _valueToWaitFor);
}

//bool CommandQueue::FenceReached(UINT _fenceValue)
//{
//	if (_fenceValue > lastReachedValue)
//	{
//		lastReachedValue = PollCurrentFenceValue();
//	}
//
//	if (lastReachedValue >= _fenceValue)
//		return true;
//	
//	return false;
//}
//
//int CommandQueue::PollCurrentFenceValue()
//{
//	if (lastReachedValue <= fence->GetCompletedValue())
//	{
//		lastReachedValue = (UINT)fence->GetCompletedValue();
//	}
//	return lastReachedValue;
//}
