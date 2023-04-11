#pragma once
#include "CommandList.h"

/** @brief Wrapper Class for the ID3D12CommandQueue and associated handeling of GPU/CPU synchronization using the ID3D12Fence class.
*/
class CommandQueue
{
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT nextFenceValue;
	D3D12_COMMAND_LIST_TYPE type;

public:
	CommandQueue() = default;

	/**Initiates the CommandQueue object.
	@param _device ID3D12Device to use for the D3D12 resource creations.
	@param _type D3D12_COMMAND_LIST_TYPE matching the type of CommandList that is executed using this object.
	@param _prio D3D12_COMMAND_QUEUE_PRIORITY of the CommandQueue.
	@param _flags Additional D3D12_COMMAND_QUEUE_FLAGS of the CommandQueue.
	@return void
	*/
	CommandQueue(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio, D3D12_COMMAND_QUEUE_FLAGS _flags);

	/**Releases the internal ID3D12CommandQueue and ID3D12Fence objects.
	*/
	~CommandQueue() { };

	/**Initiates the CommandQueue object. 
	* Usually used whenever the CommandQueue is a non-pointer object.
	@param _device ID3D12Device to use for the D3D12 resource creations.
	@param _type D3D12_COMMAND_LIST_TYPE matching the type of CommandList that is executed using this object.
	@param _prio D3D12_COMMAND_QUEUE_PRIORITY of the CommandQueue.
	@param _flags Additional D3D12_COMMAND_QUEUE_FLAGS of the CommandQueue.
	@return void
	*/
	void Init(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type, D3D12_COMMAND_QUEUE_PRIORITY _prio, D3D12_COMMAND_QUEUE_FLAGS _flags);

	/**Executes the commands recorded on the input CommandList.
	@param _cmdList CommandList which should have it's commands executed.
	@return UINT64 To the fence value which can be used in conjunction with the CommandQueue::Flush() methods.
	*/
	UINT64 Execute(CommandList& _cmdList);

	/**Returns the internal ID3D12CommandQueue* object.
	@return ID3D12CommandQueue*
	*/
	ID3D12CommandQueue* GetQueue() { return queue.Get(); }

	/**Returns the D3D12_COMMAND_LIST_TYPE of the CommandQueue.
	@return D3D12_COMMAND_LIST_TYPE
	*/
	D3D12_COMMAND_LIST_TYPE GetType() const { return type; }

	/**Stalls the CPU until the internal ID3D12Fence has been signaled the input value.
	@param _valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void StallCPU(int _valueToWaitFor);

	/**Stalls this CommandQueue on the GPU-side until the input CommandQueues' ID3D12Fence has been signaled the input value.
	* The input value is returned using CommandQueue::Execute() with the input CommandQueue.
	@param _other CommandQueue to add a GPU-side wait for.
	@param _valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void WaitForOther(CommandQueue& _other, int _valueToWaitFor);

	/**Stalls this CommandQueue on the GPU-side until this CommandQueue fence has been signaled the input value.
	* The input value is returned using CommandQueue::Execute() with the CommandQueue.
	@param _valueToWaitFor Value to wait for. The value is returned from CommandQueue::Execute().
	@return void
	*/
	void WaitForFence(int _valueToWaitFor);
};

