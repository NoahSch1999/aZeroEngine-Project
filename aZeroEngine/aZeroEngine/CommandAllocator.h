#pragma once
#include "directx/d3dx12.h"

/** @brief Wrapper Class for the ID3D12CommandAllocator.
*/
class CommandAllocator
{
private:
	D3D12_COMMAND_LIST_TYPE type;
	ID3D12CommandAllocator* allocator;

public:
	CommandAllocator() = default;

	/**Initiates the allocator as the input argument D3D12_COMMAND_LIST_TYPE
	@param _device ID3D12Device to create the allocator with.
	@param _type D3D12_COMMAND_LIST_TYPE to set create the allocator as.
	@return void
	*/
	CommandAllocator(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type);

	/**Releases the internal ID3D12CommandAllocator object.
	*/
	~CommandAllocator();

	/**Initiates the allocator as the input argument D3D12_COMMAND_LIST_TYPE
	* Usually used whenever the CommandAllocator is a non-pointer object.
	@param _device ID3D12Device to create the allocator with.
	@param _type D3D12_COMMAND_LIST_TYPE to set create the allocator as.
	@return void
	*/
	void Init(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _type);

	/**Returns the D3D12_COMMAND_LIST_TYPE of the ID3D12CommandAllocator.
	@return D3D12_COMMAND_LIST_TYPE
	*/
	D3D12_COMMAND_LIST_TYPE GetType()const { return type; }

	/**Returns the internal ID3D12CommandAllocator* object.
	@return ID3D12CommandAllocator*
	*/
	ID3D12CommandAllocator* GetAllocator() { return allocator; }

	/**Resets the ID3D12CommandAllocator
	@return void
	*/
	void Reset() { allocator->Reset(); }
};

