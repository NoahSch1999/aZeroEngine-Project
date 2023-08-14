#pragma once
#include "Shader.h"
#include "InputLayout.h"
#include "RootSignature.h"
#include "GraphicsContextHandle.h"

class GraphicsPipeline
{
public:
	struct Description
	{
		struct Shaders
		{
			std::shared_ptr<Shader> m_vertexShader = nullptr;
			std::shared_ptr<Shader> m_domainShader = nullptr;
			std::shared_ptr<Shader> m_hullShader = nullptr;
			std::shared_ptr<Shader> m_geometryShader = nullptr;
			std::shared_ptr<Shader> m_pixelShader = nullptr;
		};
		UINT m_msaaSampleCount = 1;
		DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_FORCE_UINT;
		std::vector<DXGI_FORMAT> m_renderTargetFormats;
		Shaders m_shaders;
		InputLayout m_inputLayout;
		D3D12_RASTERIZER_DESC m_rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		D3D12_BLEND_DESC m_blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		D3D12_DEPTH_STENCIL_DESC m_depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		RootSignature m_rootSignature;
	};

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDescription;
	Description m_description;

public:
	GraphicsPipeline() = default;
	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline operator=(const GraphicsPipeline&) = delete;

	GraphicsPipeline(GraphicsPipeline&& other) noexcept
	{
		m_description = other.m_description;
		m_pso = other.m_pso;
		m_psoDescription = other.m_psoDescription;
	}

	GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept
	{
		if (this != &other)
		{
			m_description = other.m_description;
			m_pso = other.m_pso;
			m_psoDescription = other.m_psoDescription;
		}
		return *this;
	}

	GraphicsPipeline(ID3D12Device* device, const Description& description)
	{
		this->create(device, description);
	}

	void create(ID3D12Device* device, const Description& description)
	{
		ZeroMemory(&m_psoDescription, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		m_description = description;

		assert(m_description.m_shaders.m_vertexShader);

		if (m_description.m_shaders.m_domainShader)
		{
			m_psoDescription.DS = {
				reinterpret_cast<BYTE*>(m_description.m_shaders.m_domainShader->getShaderBlob()->GetBufferPointer()),
				m_description.m_shaders.m_domainShader->getShaderBlob()->GetBufferSize()
			};
		}

		if (m_description.m_shaders.m_hullShader)
		{
			m_psoDescription.HS = {
				reinterpret_cast<BYTE*>(m_description.m_shaders.m_hullShader->getShaderBlob()->GetBufferPointer()),
				m_description.m_shaders.m_hullShader->getShaderBlob()->GetBufferSize()
			};
		}

		if (m_description.m_shaders.m_geometryShader)
		{
			m_psoDescription.GS = {
				reinterpret_cast<BYTE*>(m_description.m_shaders.m_geometryShader->getShaderBlob()->GetBufferPointer()),
				m_description.m_shaders.m_geometryShader->getShaderBlob()->GetBufferSize()
			};
		}

		if (m_description.m_shaders.m_pixelShader)
		{
			m_psoDescription.PS = {
				reinterpret_cast<BYTE*>(m_description.m_shaders.m_pixelShader->getShaderBlob()->GetBufferPointer()),
				m_description.m_shaders.m_pixelShader->getShaderBlob()->GetBufferSize()
			};
		}

		m_psoDescription.VS = {
				reinterpret_cast<BYTE*>(m_description.m_shaders.m_vertexShader->getShaderBlob()->GetBufferPointer()),
				m_description.m_shaders.m_vertexShader->getShaderBlob()->GetBufferSize()
		};

		m_psoDescription.RasterizerState = m_description.m_rasterizerDesc;
		m_psoDescription.BlendState = m_description.m_blendDesc;
		m_psoDescription.DepthStencilState = m_description.m_depthStencilDesc;
		m_psoDescription.PrimitiveTopologyType = m_description.m_topologyType;
		m_psoDescription.SampleDesc.Count = m_description.m_msaaSampleCount;
		m_psoDescription.SampleDesc.Quality = 0;
		m_psoDescription.SampleMask = UINT_MAX;
		m_psoDescription.DSVFormat = m_description.m_depthStencilFormat;

		// Gotten from shader reflection
		m_psoDescription.InputLayout = { m_description.m_inputLayout.getDescription(), m_description.m_inputLayout.getNumElements() };
		m_psoDescription.pRootSignature = m_description.m_rootSignature.getSignature();
		m_psoDescription.NumRenderTargets = m_description.m_renderTargetFormats.size();

		for (int i = 0; i < m_description.m_renderTargetFormats.size(); i++)
		{
			m_psoDescription.RTVFormats[i] = m_description.m_renderTargetFormats[i];
		}

		HRESULT hr = device->CreateGraphicsPipelineState(&m_psoDescription, IID_PPV_ARGS(m_pso.GetAddressOf()));
		if (FAILED(hr))
			throw;
	}
	
	void setPipeline(GraphicsContextHandle& context)
	{
		context.setPipelineState(m_pso.Get());
		context.setGraphicsRootSignature(m_description.m_rootSignature.getSignature());
	}

	Description getDescription() const { return m_description; }
};