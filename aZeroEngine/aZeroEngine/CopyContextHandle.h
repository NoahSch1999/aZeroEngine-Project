#pragma once
#include "CommandContext.h"

/** @brief Wraps a CommandContext which is used for recording copy commands.
* Dependency injects a CommandManager instance in the constructor.
* Once an instance of this goes out of scope, it will put itself back into the CommandManager as an available CopyContextHandle to be requested.
*/
class CopyContextHandle
{
	friend class CommandManager;

private:
	CommandManager* m_manager = nullptr;
	std::shared_ptr<CommandContext> m_context = nullptr;
	void SetLastFence(UINT64 lastFence) { m_context->m_lastSignal = lastFence; }

public:
	CopyContextHandle() = delete;
	CopyContextHandle(const CopyContextHandle&) = delete;
	CopyContextHandle& operator=(const CopyContextHandle) = delete;
	CopyContextHandle& operator=(CopyContextHandle&&) = delete;

	CopyContextHandle(CommandManager* manager, std::shared_ptr<CommandContext> context)
		:m_manager(manager), m_context(context) { }

	CopyContextHandle(CopyContextHandle&& other) noexcept;
	~CopyContextHandle() noexcept;

	/**Returns the internal CommandList object.
	@return CommandList&
	*/
	ID3D12GraphicsCommandList* GetList() { return static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()); }

	// -------------------------------------------------------------------------------------------------------------------
	// WRAPPER METHODS
	// --------------------------

	/**Sets the current DescriptorHeap objects on the CommandList.
	@param numHeaps Number of DescriptorHeap objects in the array that the _heaps pointer points to
	@param heaps A pointer to an array of DescriptorHeap objects
	@return void
	*/
	void SetDescriptorHeaps(UINT numHeaps, ID3D12DescriptorHeap* const* heaps)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetDescriptorHeaps(numHeaps, heaps);
	}

	/**TO ADD
	@param 
	@return
	*/
	void UpdateSubresource(ID3D12Resource* const dest, ID3D12Resource* const src, UINT64 srcOffset, 
		UINT64 rowPitch, UINT64 slicePitch, const void* const data)
	{
		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = data;
		sData.RowPitch = rowPitch;
		sData.SlicePitch = slicePitch;
		UpdateSubresources(static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()), dest, src, 0, 0, 1, &sData);
	}

	/**TO ADD
	@param
	@return
	*/
	void CopyBufferRegion(ID3D12Resource* const destBuffer, UINT64 destOffset, ID3D12Resource* const srcBuffer, UINT64 srcOffset, UINT64 numBytes)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyBufferRegion(destBuffer, destOffset, srcBuffer, srcOffset, numBytes);
	}
};