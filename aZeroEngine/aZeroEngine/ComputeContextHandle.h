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
	void SetLastFence(UINT64 lastFence) { m_context->m_lastSignal = lastFence; }

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
};
