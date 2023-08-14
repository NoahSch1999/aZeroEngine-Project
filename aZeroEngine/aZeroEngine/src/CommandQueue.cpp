#include "CommandQueue.h"

void CommandQueue::init(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY prio = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE)
{
	m_type = type;

	D3D12_COMMAND_QUEUE_DESC desc{ m_type, prio, flags, 0 };
	if(FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_queue.GetAddressOf()))))
		throw;

	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()))))
		throw;

	if (FAILED(m_fence->Signal(0)))
		throw;
}

UINT64 CommandQueue::execute(ID3D12CommandList* commandLists, UINT numLists)
{
	m_queue->ExecuteCommandLists(numLists, &commandLists);

	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue->Signal(m_fence.Get(), m_nextFenceValue);
	m_lastFenceValue = m_nextFenceValue;
	return m_nextFenceValue++; // Postincrements the value. This means that the nextFenceValue that is returned equals nextFenceValue without ++.
}

UINT64 CommandQueue::execute(ID3D12CommandList* commandList)
{
	ID3D12CommandList* commandLists = { commandList };
	m_queue->ExecuteCommandLists(1, &commandLists);

	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue->Signal(m_fence.Get(), m_nextFenceValue);
	m_lastFenceValue = m_nextFenceValue;
	return m_nextFenceValue++;
}

// Waits for the signaled value returned from CommandQueue::Execute()
void CommandQueue::flushCPU(UINT64 valueToWaitFor)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	// Get last fence value reached 
	// The fence is automatically updated whenever the ID3D12CommandQueue reaches a signal command for the fence.
	UINT64 currentFenceValue = m_fence->GetCompletedValue();

	// Check if the fence has reached _valueToWaitFor. _valueToWaitFor has to have been set using ID3D12CommandQueue::Signal()
	// Simply return if the fence value has been reached since nothing on the queue is pending that was executed before _valueToWaitFor was signaled for the fence.
	if (valueToWaitFor <= currentFenceValue)
	{
		return;
	}
	
	// Set an event handle that will be notified, and thus stop locking, once the fence has reached _valueToWaitFor.
	// Nullptr will result in an INFINITE wait until the fence has been signaled.
	m_fence->SetEventOnCompletion(valueToWaitFor, nullptr);

	return;
}

void CommandQueue::waitForOther(CommandQueue& other, UINT64 valueToWaitFor)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue->Wait(other.m_fence.Get(), valueToWaitFor);
}

void CommandQueue::waitForFence(UINT64 valueToWaitFor)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue->Wait(m_fence.Get(), valueToWaitFor);
}

UINT64 CommandQueue::getLastSignalValue()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_lastFenceValue;
}

void CommandQueue::signal()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue->Signal(m_fence.Get(), m_nextFenceValue);
	m_lastFenceValue = m_nextFenceValue;
	m_nextFenceValue++;
}
