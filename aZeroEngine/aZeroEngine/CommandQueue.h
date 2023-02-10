#pragma once
#include "CommandList.h"

/** @brief Wrapper Class for the ID3D12CommandQueue and associated handeling of GPU/CPU synchronization using the ID3D12Fence class.
*/
class CommandQueue
{
private:
	bool FenceReached(UINT _fenceValue);
	int PollCurrentFenceValue();

	ID3D12CommandQueue* queue;
	ID3D12Fence* fence;
	UINT nextFenceValue;
	UINT lastReachedValue;
	D3D12_COMMAND_LIST_TYPE type;
	HANDLE eventHandle;

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
	~CommandQueue();

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
	@return int To the fence value which can be used in conjunction with the CommandQueue::Flush() methods.
	*/
	int Execute(CommandList& _cmdList);

	/**Stalls the CPU until the recorded commands on the CommandList has been executed using CommandQueue::Execute().
	* The input fence value is returned from CommandQueue::Execute() and should be stored in order to input it as an argument to this method.
	@param _fenceValue Fence value recorded within a CommandList that CPU should wait for.
	@param _allocator Allocator which was used to create the input CommandList. It will be reset using CommandAllocator::Reset() whenever the CPU stall is completed.
	@param _cmdList CommandList that contains the commands executed using CommandQueue::Execute(). It will be reset using CommandList::Reset() whenever the CPU stall is completed.
	@return void
	*/
	void Flush(UINT _fenceValue, CommandAllocator& _allocator, CommandList& _cmdList);

	/**Stalls the CPU until the recorded commands on the CommandList has been executed using CommandQueue::Execute().
	* The input fence value is returned from CommandQueue::Execute() and should be stored in order to input it as an argument to this method.
	@param _fenceValue Fence value recorded within a CommandList that CPU should wait for.
	@return void
	*/
	void Flush(UINT _fenceValue);

	/**Returns the internal ID3D12CommandQueue* object.
	@return ID3D12CommandQueue*
	*/
	ID3D12CommandQueue* GetQueue() { return queue; }

	/**Returns the D3D12_COMMAND_LIST_TYPE of the CommandQueue.
	@return D3D12_COMMAND_LIST_TYPE
	*/
	D3D12_COMMAND_LIST_TYPE GetType() const { return type; }
};

