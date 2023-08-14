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
	void setLastFence(UINT64 lastFence) { m_context->m_lastSignal = lastFence; }

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
	ID3D12GraphicsCommandList* getList() { return static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()); }

	// -------------------------------------------------------------------------------------------------------------------
	// WRAPPER METHODS
	// --------------------------

	/**Sets the current DescriptorHeap objects on the CommandList.
	@param numHeaps Number of DescriptorHeap objects in the array that the _heaps pointer points to
	@param heaps A pointer to an array of DescriptorHeap objects
	@return void
	*/
	void setDescriptorHeaps(UINT numHeaps, ID3D12DescriptorHeap* const* heaps)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetDescriptorHeaps(numHeaps, heaps);
	}

	/**TO ADD
	@param 
	@return
	*/
	void updateSubresource(ID3D12Resource* const dest, ID3D12Resource* const src, UINT64 srcOffset, 
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
	void copyBufferRegion(ID3D12Resource* const destBuffer, UINT64 destOffset, ID3D12Resource* const srcBuffer, UINT64 srcOffset, UINT64 numBytes)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyBufferRegion(destBuffer, destOffset, srcBuffer, srcOffset, numBytes);
	}

	void copyResource(ID3D12Resource* const dest, ID3D12Resource* const src)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyResource(dest, src);
	}

	void copyTextureToBuffer(ID3D12Device* const device, const Texture& src, const Buffer& dst, const DXM::Vector2& srcOffset, UINT dstOffset, const DXM::Vector2& copyDimensions)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
		UINT numRows = 0;
		UINT64 rowSize = 0;
		UINT64 totalSize = 0;

		D3D12_RESOURCE_DESC desc = src.getGPUResource()->GetDesc();
		desc.Width = copyDimensions.x;
		desc.Height = copyDimensions.y;
		device->GetCopyableFootprints(&desc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

		D3D12_TEXTURE_COPY_LOCATION dest; // Readback
		dest.pResource = dst.getResource();
		dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint = footPrint[0];

		D3D12_TEXTURE_COPY_LOCATION source; // GPUOnly
		source.pResource = src.getGPUResource();
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = 0;

		UINT dstX = dstOffset;
		UINT dstY = 0;
		UINT dstZ = 0;
		D3D12_BOX srcBox;
		srcBox.back = 1;
		srcBox.front = 0;
		srcBox.left = srcOffset.x;
		srcBox.right = srcOffset.x + copyDimensions.x;
		srcBox.top = srcOffset.y;
		srcBox.bottom = srcOffset.y + copyDimensions.y;

		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyTextureRegion(&dest, dstX, dstY, dstZ, &source, &srcBox);
	}

	void copyTextureToBuffer(ID3D12Device* const device,
		const Texture& src, const Texture& dst, const DXM::Vector2& srcOffset, const DXM::Vector2& dstOffset,
		const DXM::Vector2& copyDimensions)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
		UINT numRows = 0;
		UINT64 rowSize = 0;
		UINT64 totalSize = 0;

		D3D12_RESOURCE_DESC desc = src.getGPUResource()->GetDesc();
		desc.Width = copyDimensions.x;
		desc.Height = copyDimensions.y;
		device->GetCopyableFootprints(&desc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

		D3D12_TEXTURE_COPY_LOCATION dest;
		dest.pResource = dst.getGPUResource();
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dest.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION source;
		source.pResource = src.getGPUResource();
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = 0;

		UINT dstX = dstOffset.x;
		UINT dstY = dstOffset.y;
		UINT dstZ = 0;
		D3D12_BOX srcBox;
		srcBox.back = 1;
		srcBox.front = 0;
		srcBox.left = srcOffset.x;
		srcBox.right = srcOffset.x + copyDimensions.x;
		srcBox.top = srcOffset.y;
		srcBox.bottom = srcOffset.y + copyDimensions.y;

		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyTextureRegion(&dest, dstX, dstY, dstZ, &source, &srcBox);
	}

	void transitionTexture(Texture& texture, D3D12_RESOURCE_STATES newState)
	{
		texture.transition(static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()), newState);
	}
};