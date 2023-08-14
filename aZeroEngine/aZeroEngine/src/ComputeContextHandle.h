#pragma once
#include "CommandContext.h"

/** @brief Wraps a CommandContext which is used for recording compute commands.
* Dependency injects a CommandManager instance in the constructor.
* Once an instance of this goes out of scope, it will put itself back into the CommandManager as an available ComputeContextHandle to be requested.
*/
class ComputeContextHandle
{
	friend class CommandManager;

private:
	CommandManager* m_manager = nullptr;
	std::shared_ptr<CommandContext> m_context = nullptr;
	void setLastFence(UINT64 lastFence) { m_context->m_lastSignal = lastFence; }

public:
	ComputeContextHandle() = delete;
	ComputeContextHandle(const ComputeContextHandle&) = delete;
	ComputeContextHandle& operator=(const ComputeContextHandle) = delete;
	ComputeContextHandle& operator=(ComputeContextHandle&&) = delete;

	ComputeContextHandle(CommandManager* manager, std::shared_ptr<CommandContext> context)
		:m_manager(manager), m_context(context) { }

	ComputeContextHandle(ComputeContextHandle&& other) noexcept;
	~ComputeContextHandle() noexcept;

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

	void setPipelineState(ID3D12PipelineState* const pipelineState)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetPipelineState(pipelineState);
	}

	void setComputeRootSignature(ID3D12RootSignature* const rootSignature)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRootSignature(rootSignature);
	}

	void setCompute32BitRootConstant(UINT index, UINT data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRoot32BitConstant(index, data, offsetIn32BitValues);
	}

	void setCompute32BitRootConstants(UINT index, UINT numValues, const void* data, UINT offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRoot32BitConstants(index, numValues, data, offsetIn32BitValues);
	}

	void setComputeShaderResourceView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetComputeRootShaderResourceView(index, virtualAddress);
	}

	void setUnorderedAccessView(UINT index, const D3D12_GPU_VIRTUAL_ADDRESS virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->SetGraphicsRootUnorderedAccessView(index, virtualAddress);
	}

	void dispatch(UINT tGroupX, UINT tGroupY, UINT tGroupZ)
	{
		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->Dispatch(tGroupX, tGroupY, tGroupZ);
	}

	void transitionTexture(Texture& texture, D3D12_RESOURCE_STATES newState)
	{
		texture.transition(static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get()), newState);
	}

	void clearUnorderedAccessViewFloat(Texture& uavTexture, const FLOAT* color)
	{

		static_cast<ID3D12GraphicsCommandList*>(m_context->m_commandList.Get())->ClearUnorderedAccessViewFloat(
			uavTexture.getUAVHandle().getGPUHandle(), uavTexture.getUAVHandle().getCPUHandle(), uavTexture.getGPUResource(),
			color, 0, nullptr);
	}
};
