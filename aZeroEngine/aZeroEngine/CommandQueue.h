#pragma once
#include "directx/d3dx12.h"
#include <mutex>

/** @brief Wraps the functionality for the ID3D12CommandQueue and ID3D12Fence classes.
*/
class CommandQueue
{
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_NONE;
	UINT m_nextFenceValue = 0;
	UINT m_lastFenceValue = 0;
	std::mutex m_mutex;

public:
	CommandQueue() = default;
	~CommandQueue() = default;

	/**Initiates the CommandQueue object.
	@param device ID3D12Device to use for the D3D12 resource creations
	@param type D3D12_COMMAND_LIST_TYPE matching the type of CommandList that is executed using this object
	@param prio D3D12_COMMAND_QUEUE_PRIORITY of the CommandQueue
	@param flags Additional D3D12_COMMAND_QUEUE_FLAGS of the CommandQueue
	@return void
	*/
	void Init(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY prio, D3D12_COMMAND_QUEUE_FLAGS flags);

	/**Executes the commands recorded on the input ID3D12CommandList and returns the ID3D12Fence signal value for the execution.
	@param commandList ID3D12CommandList* which should have it's commands executed
	@return UINT64
	*/
	UINT64 Execute(ID3D12CommandList* commandList);

	/**Executes the commands recorded on the input array of ID3D12CommandList and returns the ID3D12Fence signal value for the execution.
	@param commandList ID3D12CommandList* which should have it's commands executed
	@param numLists Number of ID3D12CommandList in the array that the pointer references
	@return UINT64
	*/
	UINT64 Execute(ID3D12CommandList* commandLists, UINT numLists);

	/**Returns the internal ID3D12CommandQueue* object.
	@return ID3D12CommandQueue*
	*/
	ID3D12CommandQueue* GetQueue() { return m_queue.Get(); }

	/**Returns the D3D12_COMMAND_LIST_TYPE of the CommandQueue.
	@return D3D12_COMMAND_LIST_TYPE
	*/
	D3D12_COMMAND_LIST_TYPE GetType() const { return m_type; }

	/**Stalls the CPU until the internal ID3D12Fence has been signaled the input value.
	@param valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void StallCPU(UINT64 valueToWaitFor);

	/**Stalls this CommandQueue on the GPU-side until the input CommandQueues' ID3D12Fence has signaled the input value.
	* The input value is returned using CommandQueue::Execute() with the input CommandQueue.
	@param other CommandQueue to add a GPU-side wait for.
	@param valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void WaitForOther(CommandQueue& other, UINT64 valueToWaitFor);

	/**Stalls this CommandQueue on the GPU-side until this CommandQueue fence has been signaled the input value.
	* The input value is returned using CommandQueue::Execute() with the CommandQueue.
	@param valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void WaitForFence(UINT64 valueToWaitFor);

	/**Returns the last signaled value from the internal ID3D12Fence.
	@return UINT64
	*/
	UINT64 GetLastSignalValue();
};

