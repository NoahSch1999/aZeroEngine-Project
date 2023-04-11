#include "CommandQueue.h"

CommandQueue::CommandQueue(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	D3D12_COMMAND_QUEUE_FLAGS _flags = D3D12_COMMAND_QUEUE_FLAG_NONE)
{
	D3D12_COMMAND_QUEUE_DESC desc{ _type, _prio, _flags, 0 };
	HRESULT hr = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(queue.GetAddressOf()));
	if (FAILED(hr))
		throw;

	hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	if (FAILED(hr))
		throw;

	hr = fence->Signal(0);
	if (FAILED(hr))
		throw;

	type = _type;
	nextFenceValue = 0;
}

void CommandQueue::Init(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	D3D12_COMMAND_QUEUE_FLAGS _flags = D3D12_COMMAND_QUEUE_FLAG_NONE)
{
	D3D12_COMMAND_QUEUE_DESC desc{ _type, _prio, _flags, 0 };
	HRESULT hr = _device->CreateCommandQueue(&desc, IID_PPV_ARGS(queue.GetAddressOf()));
	if (FAILED(hr))
		throw;

	hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	if (FAILED(hr))
		throw;

	hr = fence->Signal(0);
	if (FAILED(hr))
		throw;

	type = _type;
	nextFenceValue = 0;
}

UINT64 CommandQueue::Execute(CommandList& _cmdList)
{
	// Close the graphics list to enable execution
	_cmdList.CloseGraphic();

	// Execute the command list
	ID3D12CommandList* cmdLists = { _cmdList.GetBaseList() };
	queue->ExecuteCommandLists(1, &cmdLists);

	// Set a command queue signal for the fence and then return it.
	queue->Signal(fence.Get(), nextFenceValue);
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
	queue->Wait(_other.fence.Get(), _valueToWaitFor);
	//_other.WaitForValue(_valueToWaitFor);
}

void CommandQueue::WaitForFence(int _valueToWaitFor)
{
	queue->Wait(fence.Get(), _valueToWaitFor);
}