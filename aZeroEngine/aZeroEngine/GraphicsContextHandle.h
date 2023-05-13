#pragma once
#include "CommandContext.h"
#include "Texture.h"

/** @brief Wraps a CommandContext which is used for recording graphics/direct commands.
* Dependency injects a CommandManager instance in the constructor.
* Once an instance of this goes out of scope, it will put itself back into the CommandManager as an available GraphicsContextHandle to be requested.
*/
class GraphicsContextHandle
{
	friend class CommandManager;

private:
	CommandManager* m_manager = nullptr;
	std::shared_ptr<CommandContext> m_context = nullptr;
	void setLastFence(UINT64 lastFence) { m_context->m_lastSignal = lastFence; }

public:
	GraphicsContextHandle() = delete;
	GraphicsContextHandle(const GraphicsContextHandle&) = delete;
	GraphicsContextHandle& operator=(const GraphicsContextHandle) = delete;
	GraphicsContextHandle& operator=(GraphicsContextHandle&&) = delete;

	GraphicsContextHandle(CommandManager* manager, std::shared_ptr<CommandContext> context)
		:m_manager(manager), m_context(context) { }

	GraphicsContextHandle(GraphicsContextHandle&& _other) noexcept;

	~GraphicsContextHandle() noexcept;

	/**Returns the internal ID3D12GraphicsCommandList object.
	@return ID3D12GraphicsCommandList*
	*/
	ID3D12GraphicsCommandList* getList() { return static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()); }

	// -------------------------------------------------------------------------------------------------------------------
	// WRAPPER METHODS
	// --------------------------
	
	/**Sets the current DescriptorHeap objects on the CommandList.
	@param _numHeaps Number of DescriptorHeap objects in the array that the _heaps pointer points to
	@param _heaps A pointer to an array of DescriptorHeap objects
	@return void
	*/
	void setDescriptorHeaps(UINT numHeaps, ID3D12DescriptorHeap* const* heaps)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetDescriptorHeaps(numHeaps, heaps);
	}

	void drawInstanced(UINT vertexCount, UINT instanceCount, UINT startVertexIndex, UINT startInstanceLocation)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->DrawInstanced(vertexCount, instanceCount, startVertexIndex, startInstanceLocation);
	}

	void drawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation,
		int baseVertexLocation, UINT startInstanceLocation)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation,
			baseVertexLocation, startInstanceLocation);
	}

	void setGraphicsRootSignature(ID3D12RootSignature* const rootSignature)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootSignature(rootSignature);
	}

	void setComputeRootSignature(ID3D12RootSignature* const rootSignature)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRootSignature(rootSignature);
	}

	void setGraphicsShaderResourceView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootShaderResourceView(index, virtualAddress);
	}

	void setComputeShaderResourceView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRootShaderResourceView(index, virtualAddress);
	}

	void setConstantBufferView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootConstantBufferView(index, virtualAddress);
	}
	void setUnorderedAccessView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootUnorderedAccessView(index, virtualAddress);
	}

	void setDescriptorTable(UINT index, const D3D12_GPU_DESCRIPTOR_HANDLE startHandle)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootDescriptorTable(index, startHandle);
	}

	void setGraphics32BitRootConstant(UINT index, UINT data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRoot32BitConstant(index, data, offsetIn32BitValues);
	}

	void setGraphics32BitRootConstants(UINT index, UINT numValues, const void* data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRoot32BitConstants(index, numValues, data, offsetIn32BitValues);
	}

	void setCompute32BitRootConstant(UINT index, UINT data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRoot32BitConstant(index, data, offsetIn32BitValues);
	}

	void setCompute32BitRootConstants(UINT index, UINT numValues, const void* data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRoot32BitConstants(index, numValues, data, offsetIn32BitValues);
	}

	void setRSViewports(UINT numViewports, const D3D12_VIEWPORT* const viewports)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->RSSetViewports(numViewports, viewports);
	}

	void setRSScizzorRects(UINT numRects, const D3D12_RECT* const scizzorRects)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->RSSetScissorRects(numRects, scizzorRects);
	}

	void setPipelineState(ID3D12PipelineState* const pipelineState)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetPipelineState(pipelineState);
	}

	void setIAIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->IASetIndexBuffer(view);
	}

	void setIAVertexBuffers(UINT startSlot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW* const views)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->IASetVertexBuffers(startSlot, numViews, views);
	}

	void setIAPrimiteTopology(const D3D12_PRIMITIVE_TOPOLOGY topologyType)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->IASetPrimitiveTopology(topologyType);
	}

	void setOMRenderTargets(UINT numRenderTargets, const D3D12_CPU_DESCRIPTOR_HANDLE* const renderTargetsDesriptors,
		bool consectiveRange, const D3D12_CPU_DESCRIPTOR_HANDLE* const depthStencilDescriptor)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->OMSetRenderTargets(numRenderTargets, renderTargetsDesriptors, consectiveRange, depthStencilDescriptor);
	}

	void clearRenderTargetView(Texture& renderTarget)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->ClearRenderTargetView(renderTarget.m_handleRTVDSV.getCPUHandle(), renderTarget.m_settings.m_clearValue.Color, 0, nullptr);
	}

	void clearDepthStencilView(Texture& depthStencil)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->ClearDepthStencilView(depthStencil.m_handleRTVDSV.getCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	}

	void copyTextureToBuffer(ID3D12Device* const device, ID3D12Resource* const srcTextureResource, ID3D12Resource* const readbackDest)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
		UINT numRows = 0;
		UINT64 rowSize = 0;
		UINT64 totalSize = 0;

		D3D12_RESOURCE_DESC desc = srcTextureResource->GetDesc();
		device->GetCopyableFootprints(&desc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

		D3D12_TEXTURE_COPY_LOCATION dest; // Readback
		dest.pResource = readbackDest;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = 0;
		dest.PlacedFootprint = footPrint[0];

		D3D12_TEXTURE_COPY_LOCATION source; // GPUOnly
		source.pResource = srcTextureResource;
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = 0;

		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);
	}

	void copyBufferRegion(ID3D12Resource* const dest, UINT destOffset, ID3D12Resource* const src, UINT srcOffset, UINT numBytes)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyBufferRegion(dest, destOffset, src, srcOffset, numBytes);
	}

	void copyResource(ID3D12Resource* const dest, ID3D12Resource* const src)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->CopyResource(dest, src);
	}

	void transitionTexture(Texture& texture, D3D12_RESOURCE_STATES newState)
	{
		texture.transition(static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()), newState);
	}

	void dispatch(UINT tGroupX, UINT tGroupY, UINT tGroupZ)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->Dispatch(tGroupX, tGroupY, tGroupZ);
	}
};