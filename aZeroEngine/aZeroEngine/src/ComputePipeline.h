#pragma once
#include "Shader.h"
#include "RootSignature.h"
#include "GraphicsContextHandle.h"
#include "ComputeContextHandle.h"

class ComputePipeline
{
public:
	struct Description
	{
		std::shared_ptr<Shader> m_shader = nullptr;
		RootSignature m_rootSignature;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso = nullptr;
	Description m_description;
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDescription;

public:

	ComputePipeline() = default;
	ComputePipeline(const ComputePipeline&) = delete;
	ComputePipeline operator=(const ComputePipeline&) = delete;

	ComputePipeline(ComputePipeline&& other) noexcept
	{
		m_description = other.m_description;
		m_pso = other.m_pso;
		m_psoDescription = other.m_psoDescription;
	}

	ComputePipeline& operator=(ComputePipeline&& other) noexcept
	{
		if (this != &other)
		{
			m_description = other.m_description;
			m_pso = other.m_pso;
			m_psoDescription = other.m_psoDescription;
		}
		return *this;
	}

	ComputePipeline(ID3D12Device* device, const Description& description)
	{
		this->create(device, description);
	}

	void create(ID3D12Device* device, const Description& description)
	{
		ZeroMemory(&m_psoDescription, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		m_description = description;
		
		m_psoDescription.CS = {
			reinterpret_cast<BYTE*>(m_description.m_shader->getShaderBlob()->GetBufferPointer()),
			m_description.m_shader->getShaderBlob()->GetBufferSize()
		};

		m_psoDescription.pRootSignature = m_description.m_rootSignature.getSignature();
		m_psoDescription.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		if (FAILED(device->CreateComputePipelineState(&m_psoDescription, IID_PPV_ARGS(m_pso.GetAddressOf()))))
			throw;
	}

	void setPipeline(GraphicsContextHandle& context)
	{
		context.setPipelineState(m_pso.Get());
		context.setComputeRootSignature(m_description.m_rootSignature.getSignature());
	}

	void setPipeline(ComputeContextHandle& context)
	{
		context.setPipelineState(m_pso.Get());
		context.setComputeRootSignature(m_description.m_rootSignature.getSignature());
	}

	Description getDescription() const { return m_description; }
};