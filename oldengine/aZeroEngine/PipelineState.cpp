#include "PipelineState.h"
#include "HelperFunctions.h"

// Only works for backbuffer as render target...
void PipelineState::init(ID3D12Device* device, const PipelineStateDesc& description)
{
	// use constructor / init list...
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	Microsoft::WRL::ComPtr<ID3DBlob> vShader = Helper::LoadBlobFromFile(*description.m_vShaderPath);
	Microsoft::WRL::ComPtr<ID3DBlob> pShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> dShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> hShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> gShader = nullptr;

	if (description.m_pShaderPath)
	{
		pShader = Helper::LoadBlobFromFile(*description.m_pShaderPath);
		desc.PS = { reinterpret_cast<BYTE*>(pShader->GetBufferPointer()), pShader->GetBufferSize() };
	}
	if (description.m_dShaderPath)
	{
		dShader = Helper::LoadBlobFromFile(*description.m_dShaderPath);
		desc.DS = { reinterpret_cast<BYTE*>(dShader->GetBufferPointer()), dShader->GetBufferSize() };
	}
	if (description.m_hShaderPath)
	{
		hShader = Helper::LoadBlobFromFile(*description.m_hShaderPath);
		desc.HS = { reinterpret_cast<BYTE*>(hShader->GetBufferPointer()), hShader->GetBufferSize() };
	}
	if (description.m_gShaderPath)
	{
		gShader = Helper::LoadBlobFromFile(*description.m_gShaderPath);
		desc.GS = { reinterpret_cast<BYTE*>(gShader->GetBufferPointer()), gShader->GetBufferSize() };
	}


	desc.InputLayout = { description.m_inputLayout->getDescription(), description.m_inputLayout->getNumElements() };
	desc.pRootSignature = description.m_rootSignature->getSignature();
	desc.VS = { reinterpret_cast<BYTE*>(vShader->GetBufferPointer()), vShader->GetBufferSize() };
	desc.RasterizerState = description.m_rasterState->getDesc();

	//D3D12_RENDER_TARGET_BLEND_DESC rtvBlend;
	//ZeroMemory(&rtvBlend, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
	//rtvBlend.BlendEnable = _enableBlending;
	//rtvBlend.LogicOpEnable = false;

	//rtvBlend.SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
	//rtvBlend.DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
	//rtvBlend.BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	//rtvBlend.SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
	//rtvBlend.DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
	//rtvBlend.BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	//rtvBlend.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_SET;
	//rtvBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//D3D12_BLEND_DESC blendDesc;
	//ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	//blendDesc.AlphaToCoverageEnable = _enableBlending;
	//blendDesc.IndependentBlendEnable = false;
	//blendDesc.RenderTarget[0] = rtvBlend;

	if (description.m_blendDesc)
	{
		desc.BlendState = *description.m_blendDesc;
	}
	else
	{
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}

	if (description.m_dsvDesc)
	{
		desc.DepthStencilState = *description.m_dsvDesc;
	}
	else
	{
		desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}

	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = description.m_primitiveType;
	desc.NumRenderTargets = description.m_numRTVFormats;

	for (int i = 0; i < description.m_numRTVFormats; i++)
	{
		desc.RTVFormats[i] = description.m_rtvFormats[i];
	}

	desc.DSVFormat = description.m_dsvFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if (FAILED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()))))
		throw;
}
