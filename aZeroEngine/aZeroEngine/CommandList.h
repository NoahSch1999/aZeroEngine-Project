#pragma once
#include "CommandAllocator.h"

/** @brief Wrapper Class for the ID3D12CommandList and it's subclasses.
*/
class CommandList
{
private:
	Microsoft::WRL::ComPtr<ID3D12CommandList> basic;
	D3D12_COMMAND_LIST_TYPE type;

protected:
	ID3D12GraphicsCommandList* graphic;

public:
	CommandList() = default;

	/**Initiates the command list to the input CommandAllocator object using the input ID3D12Device*.
	@param _device ID3D12Device to create the internal ID3D12CommandList* with.
	@param _allocator CommandAllocator to link the internal ID3D12CommandList* with.
	*/
	CommandList(ID3D12Device* _device, CommandAllocator& _allocator);

	/**Releases the internal ID3D12CommandList object.
	*/
	~CommandList() { };

	/**Initiates the command list to the input CommandAllocator object using the input ID3D12Device*.
	* Usually used whenever the CommandList is a non-pointer object.
	@param _device ID3D12Device to create the internal ID3D12CommandList* with.
	@param _allocator CommandAllocator to link the internal ID3D12CommandList* with.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandAllocator& _allocator);

	/**Returns the internal ID3D12GraphicsCommandList* object.
	@return ID3D12GraphicsCommandList*
	*/
	ID3D12GraphicsCommandList* GetGraphicList() { return graphic; }
	
	/**Returns the internal ID3D12CommandList* object.
	@return ID3D12CommandList*
	*/
	ID3D12CommandList* GetBaseList() { return basic.Get(); }

	/**Returns the D3D12_COMMAND_LIST_TYPE of the CommandList.
	@return D3D12_COMMAND_LIST_TYPE
	*/
	D3D12_COMMAND_LIST_TYPE GetType() const { return type; }

	/**Resets the internal ID3D12GraphicsCommandList object.
	@param _allocator CommandAllocator associated with this CommandList.
	@param _pso Optional ID3D12PipelineState object to set when calling ID3D12GraphicsCommandList::Reset(). Default value is nullptr, which doesn't set new ID3D12PipelineState.
	@return void
	*/
	void ResetGraphic(CommandAllocator& _allocator, ID3D12PipelineState* _pso = nullptr) { graphic->Reset(_allocator.GetAllocator(), _pso); }

	/**Closes the internal ID3D12GraphicsCommandList object.
	@return void
	*/
	void CloseGraphic() { graphic->Close(); }
};

