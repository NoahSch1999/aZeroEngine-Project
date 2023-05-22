#pragma once
#include "directx/d3dx12.h"

/** @brief Wraps a ID3D12CommandList and ID3D12CommandAllocator object together.
*/
class CommandContext
{
	friend class CommandManager;
	friend class ContextHandle;
	friend class GraphicsContextHandle;
	friend class CopyContextHandle;
	friend class ComputeContextHandle;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandList> m_commandList = nullptr;

	UINT64 m_lastSignal = 0;
	bool m_dirty = false;

private:
	void startRecording(ID3D12PipelineState* resetPSO = nullptr);
	void stopRecording();
	bool hasRecorded();
	void reset();

public:

	/**Initiates the CommandContext in a non-recording state.
	@param device ID3D12Device to create the CommandContext with.
	@param type The D3D12_COMMAND_LIST_TYPE of the internal ID3D12CommandList and ID3D12CommandAllocator.
	*/
	CommandContext(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType);

	~CommandContext() = default;

};