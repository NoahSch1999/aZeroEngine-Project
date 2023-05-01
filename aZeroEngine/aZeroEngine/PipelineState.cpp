#include "PipelineState.h"
#include "HelperFunctions.h"

// Only works for backbuffer as render target...
void PipelineState::Init(ID3D12Device* device, RootSignature* rootSignature, const InputLayout& inputLayout, const RasterState& rasterState,
	UINT numRenderTargets, DXGI_FORMAT* const rtvFormats, DXGI_FORMAT dsvFormat, const std::wstring& vsPath, const std::wstring& psPath,
	const std::wstring& dsPath, const std::wstring& hsPath, const std::wstring& gsPath, bool enableBlending,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType)
{
	// use constructor / init list...
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	Microsoft::WRL::ComPtr<ID3DBlob> vShader = Helper::LoadBlobFromFile(vsPath);
	Microsoft::WRL::ComPtr<ID3DBlob> pShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> dShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> hShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> gShader = nullptr;

	if (!psPath.empty())
	{
		pShader = Helper::LoadBlobFromFile(psPath);
		desc.PS = { reinterpret_cast<BYTE*>(pShader->GetBufferPointer()), pShader->GetBufferSize() };
	}
	if (!dsPath.empty())
	{
		dShader = Helper::LoadBlobFromFile(dsPath);
		desc.DS = { reinterpret_cast<BYTE*>(dShader->GetBufferPointer()), dShader->GetBufferSize() };
	}
	if (!hsPath.empty())
	{
		hShader = Helper::LoadBlobFromFile(hsPath);
		desc.HS = { reinterpret_cast<BYTE*>(hShader->GetBufferPointer()), hShader->GetBufferSize() };
	}
	if (!gsPath.empty())
	{
		gShader = Helper::LoadBlobFromFile(gsPath);
		desc.GS = { reinterpret_cast<BYTE*>(gShader->GetBufferPointer()), gShader->GetBufferSize() };
	}


	desc.InputLayout = { &inputLayout.descs[0], 4 };
	desc.pRootSignature = rootSignature->GetSignature();
	desc.VS = { reinterpret_cast<BYTE*>(vShader->GetBufferPointer()), vShader->GetBufferSize() };
	desc.RasterizerState = rasterState.GetDesc();

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

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = primitiveType;
	desc.NumRenderTargets = numRenderTargets;

	for (int i = 0; i < numRenderTargets; i++)
	{
		desc.RTVFormats[i] = rtvFormats[i];
	}

	desc.DSVFormat = dsvFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if (FAILED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()))))
		throw;

#ifdef _DEBUG
	const std::wstring str(vsPath + psPath + dsPath + hsPath + gsPath);
	m_pipelineState->SetName(str.c_str());
#endif // _DEBUG

}
