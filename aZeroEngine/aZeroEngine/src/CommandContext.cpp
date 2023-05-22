#include "CommandContext.h"

void CommandContext::startRecording(ID3D12PipelineState* resetPSO)
{
	m_dirty = true;
	static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get())->Reset(m_allocator.Get(), resetPSO);
}

void CommandContext::stopRecording()
{
	static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get())->Close();
}

bool CommandContext::hasRecorded()
{
	return m_dirty;
}

void CommandContext::reset()
{
	m_dirty = false;
	m_allocator.Get()->Reset();
	static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get())->Reset(m_allocator.Get(), nullptr);
	static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get())->Close();
}

CommandContext::CommandContext(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType)
{
	if (FAILED(device->CreateCommandAllocator(commandType, IID_PPV_ARGS(m_allocator.GetAddressOf()))))
		throw;

	if (FAILED(device->CreateCommandList(0, commandType, m_allocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf()))))
		throw;

	static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get())->Close();
}
